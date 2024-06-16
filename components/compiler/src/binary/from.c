//
// Created by why-iskra on 12.06.2024.
//

#include <string.h>
#include "impl.h"
#include "morphinec/algorithm/crc32.h"

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

struct data {
    morphine_coroutine_t U;
    struct crc32_buf crc;
};

static inline uint8_t get_byte(struct data *D) {
    uint8_t byte = 0;

    size_t read_count = mapi_sio_read(D->U, &byte, 1);

    if (read_count != 1) {
        mapi_error(D->U, "binary corrupted");
    }

    crc32_char(&D->crc, byte);
    return byte;
}

get_type(ml_size, ml_size)
get_type(ml_line, ml_line)
get_type(ml_argument, ml_argument)
get_type(ml_integer, ml_integer)
get_type(ml_decimal, ml_decimal)
get_type(char, char)
get_type(bool, bool)
get_type(uint8_t, uint8)
get_type(uint32_t, uint32)

static const char *get_string(struct data *D) {
    ml_size size = get_ml_size(D);
    char *buffer = mapi_push_userdata_vec(D->U, "temp", size, sizeof(char));
    mapi_rotate(D->U, 2);

    for (size_t i = 0; i < size; i++) {
        buffer[i] = get_char(D);
    }

    mapi_rotate(D->U, 2);

    mapi_push_stringn(D->U, buffer, size);

    mapi_rotate(D->U, 2);
    mapi_pop(D->U, 1);

    return mapi_get_string(D->U);
}

static void load_function(struct data *D) {
    mapi_peek(D->U, 1);

    const char *name = get_string(D);

    mapi_rotate(D->U, 2);

    ml_size index = get_ml_size(D);
    ml_line line = get_ml_line(D);
    ml_size arguments_count = get_ml_size(D);
    ml_size slots_count = get_ml_size(D);
    ml_size params_count = get_ml_size(D);
    ml_size statics_count = get_ml_size(D);
    ml_size constants_count = get_ml_size(D);
    ml_size instructions_count = get_ml_size(D);

    mapi_pop(D->U, 1);

    mapi_push_function(
        D->U,
        name,
        line,
        constants_count,
        instructions_count,
        statics_count,
        arguments_count,
        slots_count,
        params_count
    );

    mapi_rotate(D->U, 2);
    mapi_pop(D->U, 1);

    mapi_vector_set(D->U, index);
}

static void load_instructions(struct data *D, ml_size index) {
    mapi_vector_get(D->U, index);
    ml_size size = mapi_instruction_size(D->U);

    mapi_peek(D->U, 2);
    for (ml_size i = 0; i < size; i++) {
        ml_line line = get_ml_line(D);
        morphine_opcode_t opcode = get_uint8(D);
        morphine_instruction_t instruction = {
            .line = line,
            .opcode = opcode,
            .argument1 = 0,
            .argument2 = 0,
            .argument3 = 0
        };

        uint8_t count = mapi_opcode_args(D->U, instruction.opcode);
        ml_argument *args = &instruction.argument1;
        for (uint8_t c = 0; c < count; c++) {
            args[c] = get_ml_argument(D);
        }

        mapi_rotate(D->U, 2);
        mapi_instruction_set(D->U, i, instruction);
        mapi_rotate(D->U, 2);
    }

    mapi_pop(D->U, 2);
}

static void load_constants(struct data *D, ml_size index) {
    mapi_vector_get(D->U, index);
    ml_size size = mapi_constant_size(D->U);

    mapi_peek(D->U, 2);
    for (ml_size i = 0; i < size; i++) {
        enum constant_tag tag = get_uint8(D);

        switch (tag) {
            case CONSTANT_TAG_NIL:
                mapi_push_nil(D->U);
                break;
            case CONSTANT_TAG_INT:
                mapi_push_integer(D->U, get_ml_integer(D));
                break;
            case CONSTANT_TAG_DEC:
                mapi_push_decimal(D->U, get_ml_decimal(D));
                break;
            case CONSTANT_TAG_STR:
                get_string(D);
                break;
            case CONSTANT_TAG_FUN: {
                ml_size fun_index = get_ml_size(D);
                mapi_pop(D->U, 1);
                mapi_rotate(D->U, 2);

                mapi_vector_get(D->U, fun_index);
                mapi_rotate(D->U, 2);
                mapi_rotate(D->U, 3);

                mapi_peek(D->U, 3);
                mapi_rotate(D->U, 2);
                break;
            }
            case CONSTANT_TAG_BOOL:
                mapi_push_boolean(D->U, get_bool(D));
                break;
            default:
                mapi_error(D->U, "unsupported constant tag");
        }

        mapi_rotate(D->U, 2);
        mapi_rotate(D->U, 3);
        mapi_constant_set(D->U, i);
        mapi_rotate(D->U, 2);
    }

    mapi_pop(D->U, 2);
}

static void check_csum(struct data *D) {
    mapi_rotate(D->U, 2);

    uint32_t expected = crc32_result(&D->crc);
    uint32_t hash = get_uint32(D);

    if (expected != hash) {
        mapi_error(D->U, "binary corrupted");
    }

    mapi_rotate(D->U, 2);
}

static void check_tag(struct data *D) {
    char buffer[sizeof(FORMAT_TAG)];
    memset(buffer, 0, sizeof(buffer));

    for (size_t i = 0; i < (sizeof(FORMAT_TAG) - 1); i++) {
        buffer[i] = get_char(D);
    }

    if (strcmp(buffer, FORMAT_TAG) != 0) {
        mapi_error(D->U, "wrong format tag");
    }
}

static void check_prob(struct data *D) {
    const char *expected_version = mapi_version();

    get_string(D);
    const char *got_version = mapi_get_string(D->U);
    if (strcmp(expected_version, got_version) != 0) {
        mapi_error(D->U, "unsupported binary version");
    }
    mapi_pop(D->U, 1);

    if (get_uint8(D) != sizeof(ml_integer)) { goto error; }
    if (get_uint8(D) != sizeof(ml_decimal)) { goto error; }
    if (get_uint8(D) != sizeof(ml_size)) { goto error; }
    if (get_uint8(D) != sizeof(ml_argument)) { goto error; }
    if (get_uint8(D) != sizeof(ml_line)) { goto error; }

    if (get_ml_integer(D) != PROB_INTEGER) { goto error; }
    if (get_ml_size(D) != PROB_SIZE) { goto error; }
    if (get_ml_decimal(D) != PROB_DECIMAL) { goto error; }

    return;
error:
    mapi_error(D->U, "unsupported binary architecture");
}

void binary_from(morphine_coroutine_t U) {
    struct data D = {
        .U = U,
        .crc = crc32_init(),
    };

    check_tag(&D);
    check_prob(&D);

    ml_size functions_count = get_ml_size(&D);
    ml_size main_index = get_ml_size(&D);

    mapi_push_vector(U, functions_count);

    for (ml_size i = 0; i < functions_count; i++) {
        load_function(&D);
    }

    for (ml_size i = 0; i < functions_count; i++) {
        load_instructions(&D, i);
    }

    for (ml_size i = 0; i < functions_count; i++) {
        load_constants(&D, i);
    }

    for (ml_size i = 0; i < functions_count; i++) {
        mapi_vector_get(U, i);
        mapi_function_complete(U);
        mapi_pop(U, 1);
    }

    check_csum(&D);

    mapi_vector_get(U, main_index);

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}
