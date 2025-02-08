//
// Created by why-iskra on 21.08.2024.
//

#include "morphine/algorithm/crc32.h"
#include "morphine/core/metatable.h"
#include "morphine/core/throw.h"
#include "morphine/gc/safe.h"
#include "morphine/misc/instruction.h"
#include "morphine/misc/packer.h"
#include "morphine/object/closure.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/userdata.h"
#include "morphine/object/vector.h"
#include <string.h>

struct data {
    morphine_instance_t I;
    struct stream *stream;
    struct vector *vector;
    struct crc32_buf crc;

    ml_size size;
    ml_size value;
};

// simple read

static inline uint8_t get_byte(struct data *D) {
    uint8_t byte = 0;
    size_t read_count = streamI_read(D->I, D->stream, &byte, 1);

    if (read_count != 1) {
        throwI_error(D->I, "corrupted packed data");
    }

    crc32_char(&D->crc, byte);

    return byte;
}

#define get_type(t, n) static inline t get_##n(struct data *D) { \
    union { \
        uint8_t raw[sizeof(t)]; \
        t result; \
    } buffer; \
    for (size_t i = 0; i < sizeof(t); i++) { \
        buffer.raw[i] = get_byte(D); \
    } \
    return buffer.result; \
}

#define get_compressed_type(t, n) static inline t get_##n(struct data *D) { \
    union { \
        uint8_t raw[sizeof(t)]; \
        t result; \
    } buffer; \
    uint8_t zeros = get_byte(D); \
    if (zeros > sizeof(t)) { throwI_error(D->I, "corrupted packed data"); } \
    for (size_t i = 0; i < sizeof(t) - zeros; i++) { buffer.raw[i] = get_byte(D); } \
    for (size_t i = sizeof(t) - zeros; i < sizeof(t); i++) { buffer.raw[i] = 0; } \
    return buffer.result; \
}

get_compressed_type(ml_line, ml_line);
get_compressed_type(ml_argument, ml_argument);
get_compressed_type(ml_integer, ml_integer);
get_compressed_type(ml_size, ml_size);
get_compressed_type(ml_decimal, ml_decimal);
get_compressed_type(ml_version, ml_version);
get_compressed_type(enum value_type, value_type);
get_compressed_type(mtype_opcode_t, opcode);
get_type(char, char);
get_type(uint8_t, uint8);
get_type(uint32_t, uint32);

static inline bool read_bool(struct data *D) {
    return get_uint8(D) > 0;
}

static struct string *read_object_string(struct data *D) {
    ml_size size = get_ml_size(D);

    gcI_safe_enter(D->I);
    struct userdata *userdata =
        gcI_safe_obj(D->I, userdata, userdataI_create_vec(D->I, size, sizeof(char), NULL, NULL));

    char *buffer = userdata->data;
    for (ml_size i = 0; i < size; i++) {
        buffer[i] = get_char(D);
    }

    struct string *string = stringI_createm(D->I, buffer, size);
    gcI_safe_exit(D->I);

    return string;
}

static struct value get_read_value(struct data *D) {
    return vectorI_get(D->I, D->vector, get_ml_size(D));
}

// complex read

static void check_format_tag(struct data *D) {
    char buffer[sizeof(FORMAT_TAG)];
    memset(buffer, 0, sizeof(buffer));

    for (size_t i = 0; i < (sizeof(FORMAT_TAG) - 1); i++) {
        buffer[i] = get_char(D);
    }

    if (strcmp(buffer, FORMAT_TAG) != 0) {
        throwI_error(D->I, "wrong format tag");
    }
}

static void check_prob(struct data *D) {
    char buffer[sizeof(MORPHINE_VERSION_NAME)];
    memset(buffer, 0, sizeof(buffer));

    for (size_t i = 0; i < (sizeof(MORPHINE_VERSION_NAME) - 1); i++) {
        buffer[i] = get_char(D);
    }

    if (strcmp(MORPHINE_VERSION_NAME, buffer) != 0) {
        throwI_error(D->I, "unsupported morphine version");
    }

    if (get_ml_version(D) != PACKER_VERSION) {
        throwI_error(D->I, "unsupported packer version");
    }

    if (get_ml_version(D) != MORPHINE_VERSION_CODE) {
        throwI_error(D->I, "unsupported morphine version code");
    }

    if (get_ml_version(D) != MORPHINE_BYTECODE_VERSION) {
        throwI_error(D->I, "unsupported bytecode version");
    }

    if (get_uint8(D) != sizeof(ml_version)) {
        goto error;
    }
    if (get_uint8(D) != sizeof(ml_integer)) {
        goto error;
    }
    if (get_uint8(D) != sizeof(ml_decimal)) {
        goto error;
    }
    if (get_uint8(D) != sizeof(ml_size)) {
        goto error;
    }
    if (get_uint8(D) != sizeof(ml_argument)) {
        goto error;
    }
    if (get_uint8(D) != sizeof(ml_line)) {
        goto error;
    }
    if (get_uint8(D) != sizeof(char)) {
        goto error;
    }
    if (get_ml_integer(D) != PROB_INTEGER) {
        goto error;
    }
    if (get_ml_size(D) != PROB_SIZE) {
        goto error;
    }
    if (get_ml_decimal(D) != PROB_DECIMAL) {
        goto error;
    }

    return;
error:
    throwI_error(D->I, "invalid packer probes");
}

static void read_head(struct data *D) {
    check_format_tag(D);
    check_prob(D);

    D->size = get_ml_size(D);
    D->value = get_ml_size(D);

    D->vector = gcI_safe_obj(D->I, vector, vectorI_create(D->I, D->size, false));
}

static void read_objects_info(struct data *D) {
    for (ml_size index = 0; index < D->size; index++) {
        ml_size vector_index = get_ml_size(D);
        enum value_type type = get_value_type(D);

        gcI_safe_enter(D->I);
        struct value preinited = valueI_nil;
        switch (type) {
            case VALUE_TYPE_NIL: preinited = valueI_nil; break;
            case VALUE_TYPE_INTEGER: preinited = valueI_integer(get_ml_integer(D)); break;
            case VALUE_TYPE_DECIMAL: preinited = valueI_decimal(get_ml_decimal(D)); break;
            case VALUE_TYPE_BOOLEAN: preinited = valueI_boolean(read_bool(D)); break;
            case VALUE_TYPE_STRING: preinited = valueI_object(read_object_string(D)); break;
            case VALUE_TYPE_TABLE: preinited = valueI_object(tableI_create(D->I)); break;
            case VALUE_TYPE_VECTOR: {
                ml_size size = get_ml_size(D);
                bool dynamic = read_bool(D);
                preinited = valueI_object(vectorI_create(D->I, size, dynamic));
                break;
            }
            case VALUE_TYPE_CLOSURE: {
                ml_size size = get_ml_size(D);
                preinited = valueI_object(closureI_packer_create(D->I, size));
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                ml_line line = get_ml_line(D);
                ml_size instructions_count = get_ml_size(D);
                ml_size constants_count = get_ml_size(D);
                ml_size slots_count = get_ml_size(D);
                ml_size params_count = get_ml_size(D);
                struct string *name = gcI_safe_obj(D->I, string, read_object_string(D));

                preinited = valueI_object(
                    functionI_create(D->I, name, line, instructions_count, constants_count, slots_count, params_count)
                );
                break;
            }
            default: throwI_error(D->I, "corrupted packed data");
        }

        gcI_safe(D->I, preinited);
        vectorI_set(D->I, D->vector, vector_index, preinited);
        gcI_safe_exit(D->I);
    }
}

static void read_objects_data(struct data *D) {
    for (ml_size index = 0; index < D->size; index++) {
        struct value value = vectorI_get(D->I, D->vector, index);
        switch (value.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING: break;
            case VALUE_TYPE_TABLE: {
                struct table *table = valueI_as_table(value);

                ml_size size = get_ml_size(D);
                for (ml_size i = 0; i < size; i++) {
                    struct value k = get_read_value(D);
                    struct value v = get_read_value(D);
                    tableI_set(D->I, table, k, v);
                }

                struct value metatable = get_read_value(D);
                metatableI_set(D->I, valueI_object(table), valueI_as_table_or_error(D->I, metatable));
                break;
            }
            case VALUE_TYPE_VECTOR: {
                struct vector *vector = valueI_as_vector(value);

                ml_size size = vectorI_size(vector);
                for (ml_size i = 0; i < size; i++) {
                    struct value v = get_read_value(D);
                    vectorI_set(D->I, vector, i, v);
                }
                break;
            }
            case VALUE_TYPE_CLOSURE: {
                struct closure *closure = valueI_as_closure(value);

                struct value callable = get_read_value(D);
                closureI_packer_init(D->I, closure, callable);

                for (ml_size i = 0; i < closure->size; i++) {
                    struct value v = get_read_value(D);
                    closureI_set(D->I, closure, i, v);
                }
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function(value);

                for (ml_size i = 0; i < function->instructions_count; i++) {
                    morphine_instruction_t instruction;
                    instruction.opcode = get_opcode(D);
                    instruction.line = get_ml_line(D);
                    instruction.argument1 = 0;
                    instruction.argument2 = 0;
                    instruction.argument3 = 0;

                    bool valid = false;
                    ml_size count = instructionI_opcode_args(instruction.opcode, &valid);

                    if (!valid) {
                        throwI_error(D->I, "corrupted instruction");
                    }

                    if (count > 0) {
                        instruction.argument1 = get_ml_argument(D);
                    }

                    if (count > 1) {
                        instruction.argument2 = get_ml_argument(D);
                    }

                    if (count > 2) {
                        instruction.argument3 = get_ml_argument(D);
                    }

                    functionI_instruction_set(D->I, function, i, instruction);
                }

                for (ml_size i = 0; i < function->constants_count; i++) {
                    struct value v = get_read_value(D);
                    functionI_constant_set(D->I, function, i, v);
                }
                break;
            }
            default: throwI_error(D->I, "corrupted packed data");
        }
    }
}

static void read_objects(struct data *D) {
    read_objects_info(D);
    read_objects_data(D);
}

static void read_tail(struct data *D) {
    uint32_t expected = crc32_result(&D->crc);
    uint32_t got = get_uint32(D);

    if (expected != got) {
        throwI_error(D->I, "corrupted packed data");
    }
}

struct value packerI_from(morphine_instance_t I, struct stream *stream) {
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(stream));

    struct data data = {
        .I = I,
        .stream = stream,
        .vector = NULL,
        .crc = crc32_init(),
        .size = 0,
        .value = 0,
    };

    read_head(&data);
    read_objects(&data);
    read_tail(&data);

    struct value value = vectorI_get(I, data.vector, data.value);

    gcI_safe_exit(I);

    return value;
}
