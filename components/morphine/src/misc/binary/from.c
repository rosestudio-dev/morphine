//
// Created by why-iskra on 21.08.2024.
//

#include "morphine/misc/packer.h"
#include "morphine/core/throw.h"
#include "morphine/gc/safe.h"
#include "morphine/object/function.h"
#include "morphine/object/closure.h"
#include "morphine/object/vector.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/userdata.h"
#include "morphine/algorithm/crc32.h"
#include <string.h>

struct data {
    morphine_instance_t I;
    struct stream *stream;
    struct vector *vector;
    struct crc32_buf crc;

    ml_size size;
    ml_size value;
};

struct packer_read {
    struct data *D;
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

get_compressed_type(ml_line, ml_line)
get_compressed_type(ml_argument, ml_argument)
get_compressed_type(ml_integer, ml_integer)
get_compressed_type(ml_size, ml_size)
get_compressed_type(ml_decimal, ml_decimal)
get_compressed_type(ml_version, ml_version)
get_compressed_type(enum value_type, value_type)
get_compressed_type(morphine_opcode_t, opcode)
get_type(char, char)
get_type(bool, bool)
get_type(uint8_t, uint8)
get_type(uint32_t, uint32)

static struct string *read_object_string(struct data *D) {
    ml_size size = get_ml_size(D);

    gcI_safe_enter(D->I);
    struct userdata *userdata = gcI_safe_obj(
        D->I, userdata, userdataI_create_vec(D->I, size, sizeof(char))
    );

    char *buffer = userdata->data;
    for (ml_size i = 0; i < size; i++) {
        buffer[i] = get_char(D);
    }

    struct string *string = stringI_createn(D->I, size, buffer);
    gcI_safe_exit(D->I);

    return string;
}

// wrappers

#define read_wrapper(t, n) t packerI_read_##n(struct packer_read *R) { return get_##n(R->D); }

read_wrapper(ml_size, ml_size)
read_wrapper(ml_line, ml_line)
read_wrapper(ml_argument, ml_argument)
read_wrapper(morphine_opcode_t, opcode)
read_wrapper(bool, bool)

struct string *packerI_read_object_string(struct packer_read *R) {
    return read_object_string(R->D);
}

struct value packerI_read_value(struct packer_read *R) {
    ml_size key_index = valueI_integer2index(R->D->I, get_ml_integer(R->D));
    return vectorI_get(R->D->I, R->D->vector, key_index);
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
    throwI_error(D->I, "invalid packer probes");
}

static void read_head(struct data *D) {
    check_format_tag(D);
    check_prob(D);

    D->size = get_ml_size(D);
    D->value = get_ml_size(D);

    D->vector = gcI_safe_obj(D->I, vector, vectorI_create(D->I, D->size));
}

static void read_objects_info(struct data *D) {
    struct packer_read read = { .D = D };

    for (ml_size index = 0; index < D->size; index++) {
        ml_size vector_index = valueI_integer2index(D->I, get_ml_integer(D));
        enum value_type type = get_value_type(D);
        switch (type) {
            case VALUE_TYPE_NIL: {
                vectorI_set(D->I, D->vector, vector_index, valueI_nil);
                break;
            }
            case VALUE_TYPE_INTEGER: {
                vectorI_set(D->I, D->vector, vector_index, valueI_integer(get_ml_integer(D)));
                break;
            }
            case VALUE_TYPE_DECIMAL: {
                vectorI_set(D->I, D->vector, vector_index, valueI_decimal(get_ml_decimal(D)));
                break;
            }
            case VALUE_TYPE_BOOLEAN: {
                vectorI_set(D->I, D->vector, vector_index, valueI_boolean(get_bool(D)));
                break;
            }
            case VALUE_TYPE_STRING: {
                gcI_safe_enter(D->I);
                struct string *string = gcI_safe_obj(D->I, string, read_object_string(D));
                vectorI_set(D->I, D->vector, vector_index, valueI_object(string));
                gcI_safe_exit(D->I);
                break;
            }
            case VALUE_TYPE_TABLE: {
                gcI_safe_enter(D->I);
                struct table *table = gcI_safe_obj(D->I, table, tableI_packer_read_info(D->I, &read));
                vectorI_set(D->I, D->vector, vector_index, valueI_object(table));
                gcI_safe_exit(D->I);
                break;
            }
            case VALUE_TYPE_VECTOR: {
                gcI_safe_enter(D->I);
                struct vector *vector = gcI_safe_obj(D->I, vector, vectorI_packer_read_info(D->I, &read));
                vectorI_set(D->I, D->vector, vector_index, valueI_object(vector));
                gcI_safe_exit(D->I);
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                gcI_safe_enter(D->I);
                struct function *function = gcI_safe_obj(D->I, function, functionI_packer_read_info(D->I, &read));
                vectorI_set(D->I, D->vector, vector_index, valueI_object(function));
                gcI_safe_exit(D->I);
                break;
            }
            case VALUE_TYPE_CLOSURE: {
                gcI_safe_enter(D->I);
                struct closure *closure = gcI_safe_obj(D->I, closure, closureI_packer_read_info(D->I, &read));
                vectorI_set(D->I, D->vector, vector_index, valueI_object(closure));
                gcI_safe_exit(D->I);
                break;
            }
            default: {
                throwI_error(D->I, "corrupted packed data");
            }
        }
    }
}

static void read_objects_data(struct data *D) {
    struct packer_read read = { .D = D };

    for (ml_size index = 0; index < D->size; index++) {
        struct value value = vectorI_get(D->I, D->vector, index);
        switch (value.type) {
            case VALUE_TYPE_NIL:
            case VALUE_TYPE_INTEGER:
            case VALUE_TYPE_DECIMAL:
            case VALUE_TYPE_BOOLEAN:
            case VALUE_TYPE_STRING:
                break;
            case VALUE_TYPE_TABLE:
                tableI_packer_read_data(D->I, valueI_as_table(value), &read);
                break;
            case VALUE_TYPE_VECTOR:
                vectorI_packer_read_data(D->I, valueI_as_vector(value), &read);
                break;
            case VALUE_TYPE_FUNCTION:
                functionI_packer_read_data(D->I, valueI_as_function(value), &read);
                break;
            case VALUE_TYPE_CLOSURE:
                closureI_packer_read_data(D->I, valueI_as_closure(value), &read);
                break;
            default:
                throwI_error(D->I, "corrupted packed data");
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
