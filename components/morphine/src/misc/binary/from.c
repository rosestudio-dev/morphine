//
// Created by why-iskra on 21.08.2024.
//

#include "morphine/misc/binary.h"
#include "morphine/core/value.h"
#include "morphine/core/throw.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/vector.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/sio.h"
#include "morphine/algorithm/crc32.h"
#include "morphine/instruction/info.h"
#include "morphine/object/userdata.h"
#include <string.h>

struct data {
    morphine_instance_t I;
    struct sio *sio;
    struct vector *vector;
    struct crc32_buf crc;

    ml_size size;
    ml_size value;
};

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
    if (zeros > sizeof(t)) { throwI_error(D->I, "corrupted binary"); } \
    for (size_t i = 0; i < sizeof(t) - zeros; i++) { buffer.raw[i] = get_byte(D); } \
    for (size_t i = sizeof(t) - zeros; i < sizeof(t); i++) { buffer.raw[i] = 0; } \
    return buffer.result; \
}

static inline uint8_t get_byte(struct data *D) {
    uint8_t byte = 0;

    size_t read_count = sioI_read(D->I, D->sio, &byte, 1);

    if (read_count != 1) {
        throwI_error(D->I, "corrupted binary");
    }

    crc32_char(&D->crc, byte);
    return byte;
}

static inline ml_size get_ml_size(struct data *D) {
    union { uint8_t raw[sizeof(ml_size)]; ml_size result; } buffer;
    uint8_t zeros = get_byte(D);
    if (zeros > sizeof(ml_size)) { throwI_error(D->I, "corrupted binary"); }
    for (size_t i = 0; i < sizeof(ml_size) - zeros; i++) { buffer.raw[i] = get_byte(D); }
    for (size_t i = sizeof(ml_size) - zeros; i < sizeof(ml_size); i++) { buffer.raw[i] = 0; }
    return buffer.result;
}
get_compressed_type(ml_line, ml_line)
get_compressed_type(ml_argument, ml_argument)
get_compressed_type(ml_integer, ml_integer)
get_compressed_type(ml_decimal, ml_decimal)
get_compressed_type(ml_version, ml_version)
get_compressed_type(enum value_type, value_type)
get_compressed_type(morphine_opcode_t, opcode)
get_type(char, char)
get_type(bool, bool)
get_type(uint8_t, uint8)
get_type(uint32_t, uint32)

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

    if (get_ml_version(D) != MORPHINEC_BINARY_VERSION) {
        throwI_error(D->I, "unsupported binary version");
    }

    if (get_ml_version(D) != MORPHINE_VERSION_CODE) {
        throwI_error(D->I, "unsupported morphine version");
    }

    if (get_ml_version(D) != MORPHINE_BYTECODE_VERSION) {
        throwI_error(D->I, "unsupported bytecode version");
    }

    if (get_uint8(D) != sizeof(ml_version)) { goto error; }
    if (get_uint8(D) != sizeof(ml_integer)) { goto error; }
    if (get_uint8(D) != sizeof(ml_decimal)) { goto error; }
    if (get_uint8(D) != sizeof(ml_size)) { goto error; }
    if (get_uint8(D) != sizeof(ml_argument)) { goto error; }
    if (get_uint8(D) != sizeof(ml_line)) { goto error; }
    if (get_uint8(D) != sizeof(char)) { goto error; }
    if (get_uint8(D) != sizeof(bool)) { goto error; }
    if (get_ml_integer(D) != PROB_INTEGER) { goto error; }
    if (get_ml_size(D) != PROB_SIZE) { goto error; }
    if (get_ml_decimal(D) != PROB_DECIMAL) { goto error; }

    return;
error:
    throwI_error(D->I, "unsupported binary architecture");
}

static void read_head(struct data *D) {
    check_format_tag(D);
    check_prob(D);

    D->size = get_ml_size(D);
    D->value = get_ml_size(D);

    D->vector = vectorI_create(D->I, D->size);
    gcI_safe_obj(D->I, objectI_cast(D->vector));
}

static struct string *read_object_string(struct data *D) {
    ml_size size = get_ml_size(D);

    struct userdata *userdata = userdataI_create_vec(D->I, size, sizeof(char));
    size_t rollback = gcI_safe_obj(D->I, objectI_cast(userdata));

    char *buffer = userdata->data;
    for (ml_size i = 0; i < size; i++) {
        buffer[i] = get_char(D);
    }

    struct string *string = stringI_createn(D->I, size, buffer);
    gcI_reset_safe(D->I, rollback);

    return string;
}

static void read_objects_info(struct data *D) {
    for (ml_size index = 0; index < D->size; index++) {
        ml_size vector_index = valueI_integer2index(D->I, get_ml_integer(D));
        enum value_type type = get_value_type(D);
        switch (type) {
            case VALUE_TYPE_NIL:
                vectorI_set(D->I, D->vector, vector_index, valueI_nil);
                continue;
            case VALUE_TYPE_INTEGER:
                vectorI_set(D->I, D->vector, vector_index, valueI_integer(get_ml_integer(D)));
                continue;
            case VALUE_TYPE_DECIMAL:
                vectorI_set(D->I, D->vector, vector_index, valueI_decimal(get_ml_decimal(D)));
                continue;
            case VALUE_TYPE_BOOLEAN:
                vectorI_set(D->I, D->vector, vector_index, valueI_boolean(get_bool(D)));
                continue;
            case VALUE_TYPE_STRING: {
                struct string *string = read_object_string(D);
                size_t rollback = gcI_safe_obj(D->I, objectI_cast(string));
                vectorI_set(D->I, D->vector, vector_index, valueI_object(string));
                gcI_reset_safe(D->I, rollback);
                continue;
            }
            case VALUE_TYPE_TABLE: {
                struct table *table = tableI_create(D->I);
                vectorI_set(D->I, D->vector, vector_index, valueI_object(table));
                continue;
            }
            case VALUE_TYPE_VECTOR: {
                ml_size size = get_ml_size(D);
                struct vector *vector = vectorI_create(D->I, size);
                vectorI_set(D->I, D->vector, vector_index, valueI_object(vector));
                continue;
            }
            case VALUE_TYPE_FUNCTION: {
                struct string *name = read_object_string(D);
                size_t rollback = gcI_safe_obj(D->I, objectI_cast(name));

                ml_line line = get_ml_line(D);
                ml_size constants_count = get_ml_size(D);
                ml_size instructions_count = get_ml_size(D);
                ml_size statics_count = get_ml_size(D);
                ml_size arguments_count = get_ml_size(D);
                ml_size slots_count = get_ml_size(D);
                ml_size params_count = get_ml_size(D);

                struct function *function = functionI_create(
                    D->I,
                    name,
                    line,
                    constants_count,
                    instructions_count,
                    statics_count,
                    arguments_count,
                    slots_count,
                    params_count
                );

                vectorI_set(D->I, D->vector, vector_index, valueI_object(function));

                gcI_reset_safe(D->I, rollback);
                continue;
            }
            default:
                throwI_error(D->I, "corrupted binary");
        }
    }
}

static void read_objects_data(struct data *D) {
    for (ml_size index = 0; index < D->size; index++) {
        struct value vector_value = vectorI_get(D->I, D->vector, index);
        switch (vector_value.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING:
                continue;
            case VALUE_TYPE_TABLE: {
                struct table *table = valueI_as_table_or_error(D->I, vector_value);
                ml_size size = get_ml_size(D);
                for (ml_size i = 0; i < size; i++) {
                    ml_size key_index = valueI_integer2index(D->I, get_ml_integer(D));
                    struct value key = vectorI_get(D->I, D->vector, key_index);

                    ml_size value_index = valueI_integer2index(D->I, get_ml_integer(D));
                    struct value value = vectorI_get(D->I, D->vector, value_index);

                    tableI_set(D->I, table, key, value);
                }
                continue;
            }
            case VALUE_TYPE_VECTOR: {
                struct vector *vector = valueI_as_vector_or_error(D->I, vector_value);
                for (ml_size i = 0; i < vectorI_size(D->I, vector); i++) {
                    ml_size value_index = valueI_integer2index(D->I, get_ml_integer(D));
                    struct value value = vectorI_get(D->I, D->vector, value_index);

                    vectorI_set(D->I, vector, i, value);
                }
                continue;
            }
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function_or_error(D->I, vector_value);

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
                        throwI_error(D->I, "corrupted binary");
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
                    ml_size value_index = valueI_integer2index(D->I, get_ml_integer(D));
                    struct value value = vectorI_get(D->I, D->vector, value_index);

                    functionI_constant_set(D->I, function, i, value);
                }

                functionI_complete(D->I, function);
                continue;
            }
            default:
                throwI_error(D->I, "corrupted binary");
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
        throwI_error(D->I, "corrupted binary");
    }
}

struct value binaryI_from(morphine_instance_t I, struct sio *sio) {
    size_t rollback = gcI_safe_obj(I, objectI_cast(sio));

    struct data data = {
        .I = I,
        .sio = sio,
        .vector = NULL,
        .crc = crc32_init(),
        .size = 0,
        .value = 0,
    };

    read_head(&data);
    read_objects(&data);
    read_tail(&data);

    struct value value = vectorI_get(I, data.vector, data.value);

    gcI_reset_safe(I, rollback);

    return value;
}
