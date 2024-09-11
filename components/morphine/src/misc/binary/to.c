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
#include "mversion.h"
#include <string.h>

struct data {
    morphine_instance_t I;
    struct sio *sio;
    struct table *table;
    struct value value;
    struct crc32_buf crc;
};

static void vectorize_append(struct data *D, struct value value) {
    bool has = false;
    tableI_get(D->I, D->table, value, &has);

    if (!has) {
        tableI_set(
            D->I,
            D->table,
            value,
            valueI_integer(tableI_size(D->I, D->table))
        );
    }
}

static void vectorize(struct data *D) {
    vectorize_append(D, D->value);

    for (ml_size index = 0; index < tableI_size(D->I, D->table); index++) {
        bool has = false;
        struct pair pair = tableI_idx_get(D->I, D->table, index, &has);

        if (!has) {
            throwI_error(D->I, "vectorize value failed");
        }

        switch (pair.key.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING:
                continue;
            case VALUE_TYPE_TABLE: {
                struct table *table = valueI_as_table(pair.key);
                for (ml_size i = 0; i < tableI_size(D->I, table); i++) {
                    bool table_has = false;
                    struct pair table_pair = tableI_idx_get(D->I, table, i, &table_has);

                    if (!table_has) {
                        throwI_error(D->I, "cannot get pair from table");
                    }

                    vectorize_append(D, table_pair.key);
                    vectorize_append(D, table_pair.value);
                }
                continue;
            }
            case VALUE_TYPE_VECTOR: {
                struct vector *vector = valueI_as_vector(pair.key);
                for (ml_size i = 0; i < vectorI_size(D->I, vector); i++) {
                    struct value value = vectorI_get(D->I, vector, i);
                    vectorize_append(D, value);
                }
                continue;
            }
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function(pair.key);
                for (ml_size i = 0; i < function->constants_count; i++) {
                    struct value value = functionI_constant_get(D->I, function, i);
                    vectorize_append(D, value);
                }
                continue;
            }
            default:
                throwI_errorf(D->I, "cannot convert %s to binary", valueI_type(D->I, pair.key, false));
        }
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

static inline void write_data(struct data *D, const uint8_t *data, size_t size) {
    size_t write_count = sioI_write(D->I, D->sio, data, size);

    if (write_count != size) {
        throwI_error(D->I, "write error");
    }

    for (size_t i = 0; i < size; i++) {
        crc32_char(&D->crc, data[i]);
    }
}

write_compress_type(ml_size, ml_size)
write_compress_type(ml_line, ml_line)
write_compress_type(ml_argument, ml_argument)
write_compress_type(ml_integer, ml_integer)
write_compress_type(ml_decimal, ml_decimal)
write_compress_type(ml_version, ml_version)
write_compress_type(enum value_type, value_type)
write_compress_type(morphine_opcode_t, opcode)
write_type(char, char)
write_type(bool, bool)
write_type(uint8_t, uint8)
write_type(uint32_t, uint32)

static void write_format_tag(struct data *D) {
    for (size_t i = 0; i < (sizeof(FORMAT_TAG) - 1); i++) {
        write_char(D, FORMAT_TAG[i]);
    }
}

static void write_prob(struct data *D) {
    for (size_t i = 0; i < (sizeof(MORPHINE_VERSION_NAME) - 1); i++) {
        write_char(D, MORPHINE_VERSION_NAME[i]);
    }

    write_ml_version(D, MORPHINEC_BINARY_VERSION);
    write_ml_version(D, MORPHINE_VERSION_CODE);
    write_ml_version(D, MORPHINE_BYTECODE_VERSION);

    write_uint8(D, sizeof(ml_version));
    write_uint8(D, sizeof(ml_integer));
    write_uint8(D, sizeof(ml_decimal));
    write_uint8(D, sizeof(ml_size));
    write_uint8(D, sizeof(ml_argument));
    write_uint8(D, sizeof(ml_line));
    write_uint8(D, sizeof(char));
    write_uint8(D, sizeof(bool));
    write_ml_integer(D, PROB_INTEGER);
    write_ml_size(D, PROB_SIZE);
    write_ml_decimal(D, PROB_DECIMAL);
}

static void write_head(struct data *D) {
    write_format_tag(D);
    write_prob(D);

    write_ml_size(D, tableI_size(D->I, D->table));
    write_ml_size(D, 0);
}

static void write_objects_info(struct data *D) {
    ml_size size = tableI_size(D->I, D->table);
    for (ml_size index = 0; index < size; index++) {
        bool has = false;
        struct pair pair = tableI_idx_get(D->I, D->table, index, &has);

        if (!has) {
            throwI_error(D->I, "vectorize value failed");
        }

        write_ml_integer(D, valueI_as_integer_or_error(D->I, pair.value));
        write_value_type(D, pair.key.type);
        switch (pair.key.type) {
            case VALUE_TYPE_NIL:
                continue;
            case VALUE_TYPE_INTEGER:
                write_ml_integer(D, valueI_as_integer(pair.key));
                continue;
            case VALUE_TYPE_DECIMAL:
                write_ml_decimal(D, valueI_as_decimal(pair.key));
                continue;
            case VALUE_TYPE_BOOLEAN:
                write_bool(D, valueI_as_boolean(pair.key));
                continue;
            case VALUE_TYPE_STRING: {
                struct string *string = valueI_as_string(pair.key);
                write_ml_size(D, string->size);
                for (ml_size i = 0; i < string->size; i++) {
                    write_char(D, string->chars[i]);
                }
                continue;
            }
            case VALUE_TYPE_TABLE:
                continue;
            case VALUE_TYPE_VECTOR:
                write_ml_size(D, vectorI_size(D->I, valueI_as_vector(pair.key)));
                continue;
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function(pair.key);

                write_ml_size(D, function->name_len);
                for (ml_size i = 0; i < function->name_len; i++) {
                    write_char(D, function->name[i]);
                }

                write_ml_line(D, function->line);
                write_ml_size(D, function->constants_count);
                write_ml_size(D, function->instructions_count);
                write_ml_size(D, function->statics_count);
                write_ml_size(D, function->arguments_count);
                write_ml_size(D, function->slots_count);
                write_ml_size(D, function->params_count);
                continue;
            }
            default:
                throwI_errorf(D->I, "cannot convert %s to binary", valueI_type(D->I, pair.key, false));
        }
    }
}

static void write_objects_data(struct data *D) {
    ml_size size = tableI_size(D->I, D->table);
    for (ml_size index = 0; index < size; index++) {
        bool has = false;
        struct pair pair = tableI_idx_get(D->I, D->table, index, &has);

        if (!has) {
            throwI_error(D->I, "vectorize value failed");
        }

        switch (pair.key.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING:
                continue;
            case VALUE_TYPE_TABLE: {
                struct table *table = valueI_as_table(pair.key);
                write_ml_size(D, tableI_size(D->I, table));
                for (ml_size i = 0; i < tableI_size(D->I, table); i++) {
                    bool table_has = false;
                    struct pair table_pair = tableI_idx_get(D->I, table, i, &table_has);

                    if (!table_has) {
                        throwI_error(D->I, "cannot get pair from table");
                    }

                    struct value key = tableI_get(D->I, D->table, table_pair.key, NULL);
                    write_ml_integer(D, valueI_as_integer_or_error(D->I, key));

                    struct value value = tableI_get(D->I, D->table, table_pair.value, NULL);
                    write_ml_integer(D, valueI_as_integer_or_error(D->I, value));
                }
                continue;
            }
            case VALUE_TYPE_VECTOR: {
                struct vector *vector = valueI_as_vector(pair.key);
                for (ml_size i = 0; i < vectorI_size(D->I, vector); i++) {
                    struct value value = vectorI_get(D->I, vector, i);
                    struct value value_index = tableI_get(D->I, D->table, value, NULL);
                    write_ml_integer(D, valueI_as_integer_or_error(D->I, value_index));
                }
                continue;
            }
            case VALUE_TYPE_FUNCTION: {
                struct function *function = valueI_as_function(pair.key);

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
                    struct value value = functionI_constant_get(D->I, function, i);
                    struct value value_index = tableI_get(D->I, D->table, value, NULL);
                    write_ml_integer(D, valueI_as_integer_or_error(D->I, value_index));
                }
                continue;
            }
            default:
                throwI_errorf(D->I, "cannot convert %s to binary", valueI_type(D->I, pair.key, false));
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

void binaryI_to(morphine_instance_t I, struct sio *sio, struct value value) {
    size_t rollback = gcI_safe_value(I, value);
    gcI_safe_obj(I, objectI_cast(sio));

    struct data data = {
        .I = I,
        .sio = sio,
        .table = NULL,
        .value = value,
        .crc = crc32_init()
    };

    data.table = tableI_create(I);
    gcI_safe_obj(I, objectI_cast(data.table));

    vectorize(&data);

    write_head(&data);
    write_objects(&data);
    write_tail(&data);

    sioI_flush(I, sio);

    gcI_reset_safe(I, rollback);
}