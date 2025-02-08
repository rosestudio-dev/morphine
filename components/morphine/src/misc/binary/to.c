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
#include "morphine/object/function.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/vector.h"

struct data {
    morphine_instance_t I;
    struct stream *stream;
    struct table *table;
    struct value value;
    struct crc32_buf crc;
};

// vectorize

static void vectorize_append(struct data *D, struct value value) {
    bool has = false;
    tableI_get(D->table, value, &has);

    if (!has) {
        struct value index = valueI_integer(tableI_size(D->table));
        tableI_set(D->I, D->table, value, index);
    }
}

// simple writes

static inline void write_data(struct data *D, const uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        uint8_t byte = data[i];
        size_t write_count = streamI_write(D->I, D->stream, &byte, sizeof(uint8_t));

        if (write_count != sizeof(uint8_t)) {
            throwI_error(D->I, "write error");
        }

        crc32_char(&D->crc, byte);
    }
}

#define write_type(t, n) static inline void write_##n(struct data *D, t value) { \
    union { \
        uint8_t raw[sizeof(t)]; \
        t value; \
    } buffer; \
    buffer.value = value; \
    write_data(D, buffer.raw, sizeof(t)); \
}

#define write_compress_type(t, n) static inline void write_##n(struct data *D, t value) { \
    union { \
        uint8_t raw[sizeof(t)]; \
        t value; \
    } buffer; \
    buffer.value = value; \
    uint8_t zeros = 0; \
    for (size_t i = 0; i < sizeof(t); i++) { \
        if (buffer.raw[sizeof(t) - i - 1] == 0) { zeros ++; } else { break; } \
    } \
    write_data(D, &zeros, sizeof(uint8_t)); \
    write_data(D, buffer.raw, sizeof(t) - zeros); \
}

write_compress_type(ml_size, ml_size);
write_compress_type(ml_line, ml_line);
write_compress_type(ml_argument, ml_argument);
write_compress_type(ml_integer, ml_integer);
write_compress_type(ml_decimal, ml_decimal);
write_compress_type(ml_version, ml_version);
write_compress_type(enum value_type, value_type);
write_compress_type(mtype_opcode_t, opcode);
write_type(char, char);
write_type(uint8_t, uint8);
write_type(uint32_t, uint32);

static inline void write_bool(struct data *D, bool value) {
    write_uint8(D, value ? 1 : 0);
}

static void write_object_string(struct data *D, struct string *string) {
    write_ml_size(D, string->size);
    for (ml_size i = 0; i < string->size; i++) {
        write_char(D, string->chars[i]);
    }
}

static void write_value(struct data *D, struct value value) {
    bool has = false;
    struct value key = tableI_get(D->table, value, &has);
    if (!has) {
        throwI_error(D->I, "unrecognized value");
    }

    write_ml_size(D, valueI_as_index_or_error(D->I, key));
}

// vectorize

static void vectorize(struct data *D) {
    vectorize_append(D, D->value);

    ml_size table_size = tableI_size(D->table);
    for (ml_size index = 0; index < table_size; index++) {
        struct value value = tableI_idx_get(D->I, D->table, index).key;
        switch (value.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING: break;
            case VALUE_TYPE_TABLE: {
                struct table *table = valueI_as_table(value);

                ml_size size = tableI_size(table);
                for(ml_size i = 0; i < size; i ++) {
                    struct pair pair = tableI_idx_get(D->I, table, i);
                    vectorize_append(D, pair.key);
                    vectorize_append(D, pair.value);
                }

                vectorize_append(D, metatableI_get(D->I, valueI_object(table)));
                break;
            }
            case VALUE_TYPE_VECTOR: {
                struct vector *vector = valueI_as_vector(value);

                ml_size size = vectorI_size(vector);
                for (ml_size i = 0; i < size; i++) {
                    vectorize_append(D, vectorI_get(D->I, vector, i));
                }
                break;
            }
            case VALUE_TYPE_CLOSURE: {
                struct closure *closure = valueI_as_closure(value);

                vectorize_append(D, closure->callable);
                for (ml_size i = 0; i < closure->size; i++) {
                    vectorize_append(D, closureI_get(D->I, closure, i));
                }
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function(value);

                for (ml_size i = 0; i < function->constants_count; i++) {
                    vectorize_append(D, functionI_constant_get(D->I, function, i));
                }
                break;
            }
            default: throwI_errorf(D->I, "%s cannot be packed", valueI_type(D->I, value, false));
        }
    }
}

// complex write

static void write_format_tag(struct data *D) {
    for (size_t i = 0; i < (sizeof(FORMAT_TAG) - 1); i++) {
        write_char(D, FORMAT_TAG[i]);
    }
}

static void write_prob(struct data *D) {
    for (size_t i = 0; i < (sizeof(MORPHINE_VERSION_NAME) - 1); i++) {
        write_char(D, MORPHINE_VERSION_NAME[i]);
    }

    write_ml_version(D, PACKER_VERSION);
    write_ml_version(D, MORPHINE_VERSION_CODE);
    write_ml_version(D, MORPHINE_BYTECODE_VERSION);

    write_uint8(D, sizeof(ml_version));
    write_uint8(D, sizeof(ml_integer));
    write_uint8(D, sizeof(ml_decimal));
    write_uint8(D, sizeof(ml_size));
    write_uint8(D, sizeof(ml_argument));
    write_uint8(D, sizeof(ml_line));
    write_uint8(D, sizeof(char));
    write_ml_integer(D, PROB_INTEGER);
    write_ml_size(D, PROB_SIZE);
    write_ml_decimal(D, PROB_DECIMAL);
}

static void write_head(struct data *D) {
    write_format_tag(D);
    write_prob(D);

    write_ml_size(D, tableI_size(D->table));
    write_ml_size(D, 0);
}

static void write_objects_info(struct data *D) {
    ml_size table_size = tableI_size(D->table);
    for (ml_size index = 0; index < table_size; index++) {
        struct pair pair = tableI_idx_get(D->I, D->table, index);
        write_ml_size(D, valueI_as_index_or_error(D->I, pair.value));

        struct value value = pair.key;
        write_value_type(D, value.type);
        switch (value.type) {
            case VALUE_TYPE_NIL: break;
            case VALUE_TYPE_INTEGER: write_ml_integer(D, valueI_as_integer(value)); break;
            case VALUE_TYPE_DECIMAL: write_ml_decimal(D, valueI_as_decimal(value)); break;
            case VALUE_TYPE_BOOLEAN: write_bool(D, valueI_as_boolean(value)); break;
            case VALUE_TYPE_STRING: write_object_string(D, valueI_as_string(value)); break;
            case VALUE_TYPE_TABLE: break;
            case VALUE_TYPE_VECTOR: {
                struct vector *vector = valueI_as_vector(value);
                write_ml_size(D, vectorI_size(vector));
                write_bool(D, vector->dynamic);
                break;
            }
            case VALUE_TYPE_CLOSURE: {
                struct closure *closure = valueI_as_closure(value);
                write_ml_size(D, closure->size);
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function(value);
                write_ml_line(D, function->line);
                write_ml_size(D, function->instructions_count);
                write_ml_size(D, function->constants_count);
                write_ml_size(D, function->slots_count);
                write_ml_size(D, function->params_count);
                write_object_string(D, function->name);
                break;
            }
            default: throwI_errorf(D->I, "%s cannot be packed", valueI_type(D->I, value, false));
        }
    }
}

static void write_objects_data(struct data *D) {
    ml_size table_size = tableI_size(D->table);
    for (ml_size index = 0; index < table_size; index++) {
        struct value value = tableI_idx_get(D->I, D->table, index).key;
        switch (value.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING: break;
            case VALUE_TYPE_TABLE: {
                struct table *table = valueI_as_table(value);
                ml_size size = tableI_size(table);

                write_ml_size(D, size);
                for(ml_size i = 0; i < size; i ++) {
                    struct pair pair = tableI_idx_get(D->I, table, i);
                    write_value(D, pair.key);
                    write_value(D, pair.value);
                }

                write_value(D, metatableI_get(D->I, valueI_object(table)));
                break;
            }
            case VALUE_TYPE_VECTOR: {
                struct vector *vector = valueI_as_vector(value);
                ml_size size = vectorI_size(vector);
                for (ml_size i = 0; i < size; i++) {
                    write_value(D, vectorI_get(D->I, vector, i));
                }
                break;
            }
            case VALUE_TYPE_CLOSURE: {
                struct closure *closure = valueI_as_closure(value);
                write_value(D, closure->callable);
                for (ml_size i = 0; i < closure->size; i++) {
                    write_value(D, closureI_get(D->I, closure, i));
                }
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function(value);

                for (ml_size i = 0; i < function->instructions_count; i++) {
                    morphine_instruction_t instruction = functionI_instruction_get(D->I, function, i);
                    write_opcode(D, instruction.opcode);
                    write_ml_line(D, instruction.line);

                    ml_size count = instructionI_opcode_args(instruction.opcode, NULL);

                    if (count > 0) {
                        write_ml_argument(D, instruction.argument1);
                    }

                    if (count > 1) {
                        write_ml_argument(D, instruction.argument2);
                    }

                    if (count > 2) {
                        write_ml_argument(D, instruction.argument3);
                    }
                }

                for (ml_size i = 0; i < function->constants_count; i++) {
                    write_value(D, functionI_constant_get(D->I, function, i));
                }
                break;
            }
            default: throwI_errorf(D->I, "%s cannot be packed", valueI_type(D->I, value, false));
        }
    }
}

static void write_objects(struct data *D) {
    write_objects_info(D);
    write_objects_data(D);
}

static void write_tail(struct data *D) {
    write_uint32(D, crc32_result(&D->crc));
}

void packerI_to(morphine_instance_t I, struct stream *stream, struct value value) {
    gcI_safe_enter(I);
    gcI_safe(I, value);
    gcI_safe(I, valueI_object(stream));

    struct data data = {
        .I = I,
        .stream = stream,
        .table = NULL,
        .value = value,
        .crc = crc32_init(),
    };

    data.table = gcI_safe_obj(I, table, tableI_create(I));

    vectorize(&data);

    write_head(&data);
    write_objects(&data);
    write_tail(&data);

    streamI_flush(I, stream);

    gcI_safe_exit(I);
}
