//
// Created by why-iskra on 21.08.2024.
//

#include "morphine/algorithm/crc32.h"
#include "morphine/core/throw.h"
#include "morphine/gc/safe.h"
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

struct packer_vectorize {
    struct data *D;
};

struct packer_write {
    struct data *D;
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
write_type(bool, bool);
write_type(uint8_t, uint8);
write_type(uint32_t, uint32);

static void write_object_string(struct data *D, struct string *string) {
    write_ml_size(D, string->size);
    for (ml_size i = 0; i < string->size; i++) {
        write_char(D, string->chars[i]);
    }
}

// wrappers

#define write_wrapper(t, n) void packerI_write_##n(struct packer_write *W, t value) { write_##n(W->D, value); }

write_wrapper(ml_size, ml_size);
write_wrapper(ml_line, ml_line);
write_wrapper(ml_argument, ml_argument);
write_wrapper(mtype_opcode_t, opcode);
write_wrapper(bool, bool);

void packerI_write_object_string(struct packer_write *W, struct string *value) {
    write_object_string(W->D, value);
}

void packerI_write_value(struct packer_write *W, struct value value) {
    bool has = false;
    struct value key = tableI_get(W->D->table, value, &has);
    if (!has) {
        throwI_error(W->D->I, "unrecognized value");
    }

    write_ml_size(W->D, valueI_as_index_or_error(W->D->I, key));
}

void packerI_vectorize_append(struct packer_vectorize *V, struct value value) {
    vectorize_append(V->D, value);
}

// vectorize

static void vectorize(struct data *D) {
    struct packer_vectorize vectorize = { .D = D };

    vectorize_append(D, D->value);

    for (ml_size index = 0; index < tableI_size(D->table); index++) {
        struct pair pair = tableI_idx_get(D->I, D->table, index);
        switch (pair.key.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING: break;
            case VALUE_TYPE_TABLE: tableI_packer_vectorize(D->I, valueI_as_table(pair.key), &vectorize); break;
            case VALUE_TYPE_VECTOR: vectorI_packer_vectorize(D->I, valueI_as_vector(pair.key), &vectorize); break;
            case VALUE_TYPE_FUNCTION: functionI_packer_vectorize(D->I, valueI_as_function(pair.key), &vectorize); break;
            case VALUE_TYPE_CLOSURE: closureI_packer_vectorize(D->I, valueI_as_closure(pair.key), &vectorize); break;
            default: throwI_errorf(D->I, "%s cannot be packed", valueI_type(D->I, pair.key, false));
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
    write_uint8(D, sizeof(bool));
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
    struct packer_write write = { .D = D };

    ml_size size = tableI_size(D->table);
    for (ml_size index = 0; index < size; index++) {
        struct pair pair = tableI_idx_get(D->I, D->table, index);
        write_ml_size(D, valueI_as_index_or_error(D->I, pair.value));
        write_value_type(D, pair.key.type);
        switch (pair.key.type) {
            case VALUE_TYPE_NIL: break;
            case VALUE_TYPE_INTEGER: write_ml_integer(D, valueI_as_integer(pair.key)); break;
            case VALUE_TYPE_DECIMAL: write_ml_decimal(D, valueI_as_decimal(pair.key)); break;
            case VALUE_TYPE_BOOLEAN: write_bool(D, valueI_as_boolean(pair.key)); break;
            case VALUE_TYPE_STRING: write_object_string(D, valueI_as_string(pair.key)); break;
            case VALUE_TYPE_TABLE: tableI_packer_write_info(D->I, valueI_as_table(pair.key), &write); break;
            case VALUE_TYPE_VECTOR: vectorI_packer_write_info(D->I, valueI_as_vector(pair.key), &write); break;
            case VALUE_TYPE_FUNCTION: functionI_packer_write_info(D->I, valueI_as_function(pair.key), &write); break;
            case VALUE_TYPE_CLOSURE: closureI_packer_write_info(D->I, valueI_as_closure(pair.key), &write); break;
            default: throwI_errorf(D->I, "%s cannot be packed", valueI_type(D->I, pair.key, false));
        }
    }
}

static void write_objects_data(struct data *D) {
    struct packer_write write = { .D = D };

    ml_size size = tableI_size(D->table);
    for (ml_size index = 0; index < size; index++) {
        struct pair pair = tableI_idx_get(D->I, D->table, index);
        switch (pair.key.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING: break;
            case VALUE_TYPE_TABLE: tableI_packer_write_data(D->I, valueI_as_table(pair.key), &write); break;
            case VALUE_TYPE_VECTOR: vectorI_packer_write_data(D->I, valueI_as_vector(pair.key), &write); break;
            case VALUE_TYPE_FUNCTION: functionI_packer_write_data(D->I, valueI_as_function(pair.key), &write); break;
            case VALUE_TYPE_CLOSURE: closureI_packer_write_data(D->I, valueI_as_closure(pair.key), &write); break;
            default: throwI_errorf(D->I, "%s cannot be packed", valueI_type(D->I, pair.key, false));
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
