//
// Created by why-iskra on 12.06.2024.
//

#include <string.h>
#include "impl.h"
#include "morphinec/algorithm/crc32.h"
#include "morphinec/rollout.h"
#include "morphinec/binary.h"

struct data {
    morphine_coroutine_t U;
    struct crc32_buf crc;
};

#define write_type(t, n) static inline void write_##n(struct data *D, t value) { \
    union { \
        uint8_t raw[sizeof(t)]; \
        t value; \
    } buffer; \
    buffer.value = value; \
    write_data(D, buffer.raw, sizeof(t)); \
}

static inline void write_data(struct data *D, const uint8_t *data, size_t size) {
    size_t write_count = mapi_sio_write(D->U, data, size);

    if (write_count != size) {
        mapi_error(D->U, "write error");
    }

    for (size_t i = 0; i < size; i++) {
        crc32_char(&D->crc, data[i]);
    }
}

write_type(ml_size, ml_size)
write_type(ml_line, ml_line)
write_type(ml_argument, ml_argument)
write_type(ml_integer, ml_integer)
write_type(ml_decimal, ml_decimal)
write_type(ml_version, ml_version)
write_type(char, char)
write_type(bool, bool)
write_type(uint8_t, uint8)
write_type(uint32_t, uint32)

static void write_string(struct data *D, const char *data, size_t size) {
    if (size > MLIMIT_SIZE_MAX) {
        mapi_error(D->U, "too big string");
    }

    write_ml_size(D, (ml_size) size);
    for (size_t i = 0; i < size; i++) {
        write_char(D, data[i]);
    }
}

static void write_format_tag(struct data *D) {
    for (size_t i = 0; i < sizeof(FORMAT_TAG) - 1; i++) {
        write_char(D, FORMAT_TAG[i]);
    }
}

static void write_prob(struct data *D) {
    const char *version = mapi_version_name();

    write_string(D, version, strlen(version));
    write_uint8(D, sizeof(ml_version));
    write_uint8(D, sizeof(ml_integer));
    write_uint8(D, sizeof(ml_decimal));
    write_uint8(D, sizeof(ml_size));
    write_uint8(D, sizeof(ml_argument));
    write_uint8(D, sizeof(ml_line));

    write_ml_version(D, mcapi_binary_version());
    write_ml_version(D, mapi_version());
    write_ml_version(D, mapi_bytecode_version());
    write_ml_integer(D, PROB_INTEGER);
    write_ml_size(D, PROB_SIZE);
    write_ml_decimal(D, PROB_DECIMAL);
}

static void write_function(struct data *D, ml_size index) {
    mapi_peek(D->U, 1);

    mapi_table_idx_getoe(D->U, index);
    ml_size idx = mapi_get_index(D->U);
    mapi_pop(D->U, 1);

    mapi_table_idx_keyoe(D->U, index);

    const char *name = mapi_function_name(D->U);
    ml_line line = mapi_function_line(D->U);
    ml_size arguments_count = mapi_function_arguments(D->U);
    ml_size slots_count = mapi_function_slots(D->U);
    ml_size params_count = mapi_function_params(D->U);
    ml_size statics_count = mapi_static_size(D->U);
    ml_size constants_count = mapi_constant_size(D->U);
    ml_size instructions_count = mapi_instruction_size(D->U);

    mapi_peek(D->U, 2);

    write_string(D, name, strlen(name));
    write_ml_size(D, idx);
    write_ml_line(D, line);
    write_ml_size(D, arguments_count);
    write_ml_size(D, slots_count);
    write_ml_size(D, params_count);
    write_ml_size(D, statics_count);
    write_ml_size(D, constants_count);
    write_ml_size(D, instructions_count);

    mapi_pop(D->U, 3);
}

static void write_instructions(struct data *D, ml_size index) {
    mapi_peek(D->U, 1);

    mapi_table_idx_keyoe(D->U, index);

    ml_size size = mapi_instruction_size(D->U);
    for (ml_size i = 0; i < size; i++) {
        morphine_instruction_t instruction = mapi_instruction_get(D->U, i);
        mapi_peek(D->U, 2);

        write_ml_line(D, instruction.line);
        write_uint8(D, (uint8_t) instruction.opcode);

        uint8_t count = mapi_opcode_args(D->U, instruction.opcode);
        ml_argument *args = &instruction.argument1;
        for (uint8_t c = 0; c < count; c++) {
            write_ml_argument(D, args[c]);
        }

        mapi_pop(D->U, 1);
    }

    mapi_pop(D->U, 2);
}

static void write_constants(struct data *D, ml_size index) {
    mapi_peek(D->U, 1);

    mapi_table_idx_keyoe(D->U, index);

    ml_size size = mapi_constant_size(D->U);
    for (ml_size i = 0; i < size; i++) {
        mapi_constant_get(D->U, i);
        const char *type = mapi_type(D->U);

        if (strcmp("nil", type) == 0) {
            mapi_peek(D->U, 3);
            write_uint8(D, CONSTANT_TAG_NIL);
        } else if (strcmp("integer", type) == 0) {
            ml_integer integer = mapi_get_integer(D->U);

            mapi_peek(D->U, 3);
            write_uint8(D, CONSTANT_TAG_INT);
            write_ml_integer(D, integer);
        } else if (strcmp("decimal", type) == 0) {
            ml_decimal decimal = mapi_get_decimal(D->U);

            mapi_peek(D->U, 3);
            write_uint8(D, CONSTANT_TAG_DEC);
            write_ml_decimal(D, decimal);
        } else if (strcmp("string", type) == 0) {
            const char *str = mapi_get_string(D->U);
            ml_size str_size = mapi_string_len(D->U);

            mapi_peek(D->U, 3);
            write_uint8(D, CONSTANT_TAG_STR);
            write_string(D, str, str_size);
        } else if (strcmp("function", type) == 0) {
            mapi_peek(D->U, 2);
            mapi_peek(D->U, 1);
            mapi_table_getoe(D->U);

            ml_size idx = mapi_get_index(D->U);
            mapi_pop(D->U, 2);

            mapi_peek(D->U, 3);
            write_uint8(D, CONSTANT_TAG_FUN);
            write_ml_size(D, idx);
        } else if (strcmp("boolean", type) == 0) {
            bool boolean = mapi_get_boolean(D->U);

            mapi_peek(D->U, 3);
            write_uint8(D, CONSTANT_TAG_BOOL);
            write_bool(D, boolean);
        } else {
            mapi_error(D->U, "unsupported constant");
        }

        mapi_pop(D->U, 2);
    }

    mapi_pop(D->U, 2);
}

static void write(morphine_coroutine_t U, ml_size count) {
    struct data D = {
        .U = U,
        .crc = crc32_init(),
    };

    write_format_tag(&D);
    write_prob(&D);

    write_ml_size(&D, count);
    write_ml_size(&D, 0);

    for (ml_size i = 0; i < count; i++) {
        write_function(&D, i);
    }

    for (ml_size i = 0; i < count; i++) {
        write_instructions(&D, i);
    }

    for (ml_size i = 0; i < count; i++) {
        write_constants(&D, i);
    }

    write_uint32(&D, crc32_result(&D.crc));
    mapi_sio_flush(U);
}

void binary_to(morphine_coroutine_t U) {
    mapi_peek(U, 1);
    mapi_peek(U, 1);

    ml_size count = mcapi_rollout(U);
    mapi_rotate(U, 2);

    write(U, count);
    mapi_pop(U, 2);
}