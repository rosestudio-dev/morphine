//
// Created by why-iskra on 31.08.2024.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/builtin.h"

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            ml_line line = 0;
            ml_size instructions_count = 0;
            ml_size constants_count = 0;
            ml_size slots_count = 0;
            ml_size params_count = 0;

            if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);

                mapi_push_string(U, "line");
                if (mapi_table_get(U)) {
                    line = mapi_get_size(U, "line");
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "instructions");
                if (mapi_table_get(U)) {
                    instructions_count = mapi_get_size(U, NULL);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "constants");
                if (mapi_table_get(U)) {
                    constants_count = mapi_get_size(U, NULL);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "slots");
                if (mapi_table_get(U)) {
                    slots_count = mapi_get_size(U, NULL);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "params");
                if (mapi_table_get(U)) {
                    params_count = mapi_get_size(U, NULL);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "name");
                if (!mapi_table_get(U)) {
                    mapi_push_string(U, "anonymous");
                }
            } else {
                maux_expect_args(U, 6);

                mapi_push_arg(U, 1);
                line = mapi_get_size(U, "line");

                mapi_push_arg(U, 2);
                instructions_count = mapi_get_size(U, NULL);

                mapi_push_arg(U, 3);
                constants_count = mapi_get_size(U, NULL);

                mapi_push_arg(U, 4);
                slots_count = mapi_get_size(U, NULL);

                mapi_push_arg(U, 5);
                params_count = mapi_get_size(U, NULL);

                mapi_push_arg(U, 0);
            }

            mapi_push_function(
                U, line,
                instructions_count,
                constants_count,
                slots_count,
                params_count
            );
            maux_nb_return();
    maux_nb_end
}

static void copy(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_function_copy(U);
            maux_nb_return();
    maux_nb_end
}

static void info(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_extract_source(U);

            ml_line line = mapi_function_line(U);
            ml_size constants_count = mapi_constant_size(U);
            ml_size instructions_count = mapi_instruction_size(U);
            ml_size slots_count = mapi_function_slots(U);
            ml_size params_count = mapi_function_params(U);
            mapi_function_name(U);

            mapi_push_table(U);

            mapi_push_string(U, "name");

            mapi_rotate(U, 3);
            mapi_rotate(U, 3);
            mapi_table_set(U);

            mapi_push_string(U, "line");
            mapi_push_integer(U, line);
            mapi_table_set(U);

            mapi_push_string(U, "constants");
            mapi_push_integer(U, constants_count);
            mapi_table_set(U);

            mapi_push_string(U, "instructions");
            mapi_push_integer(U, instructions_count);
            mapi_table_set(U);

            mapi_push_string(U, "slots");
            mapi_push_integer(U, slots_count);
            mapi_table_set(U);

            mapi_push_string(U, "params");
            mapi_push_integer(U, params_count);
            mapi_table_set(U);

            maux_nb_return();
    maux_nb_end
}

static void getconstant(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_push_arg(U, 0);
            mapi_extract_source(U);
            mapi_constant_get(U, index);
            maux_nb_return();
    maux_nb_end
}

static void setconstant(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);
            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_push_arg(U, 0);
            mapi_push_arg(U, 2);
            mapi_constant_set(U, index);
            maux_nb_leave();
    maux_nb_end
}

static void getinstruction(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");
            mapi_push_arg(U, 0);
            mapi_extract_source(U);
            morphine_instruction_t instruction = mapi_instruction_get(U, index);

            mapi_push_table(U);

            mapi_push_string(U, "opcode");
            mapi_push_string(U, maux_opcode_name(U, instruction.opcode));
            mapi_table_set(U);

            mapi_push_string(U, "line");
            mapi_push_integer(U, instruction.line);
            mapi_table_set(U);

            size_t count = mapi_opcode(U, instruction.opcode);
            for (size_t i = 0; i < MORPHINE_INSTRUCTION_ARGS_COUNT; i++) {
                mapi_push_stringf(U, "argument%zu", i);
                if (i < count) {
                    mapi_push_integer(U, instruction.arguments[i]);
                } else {
                    mapi_push_nil(U);
                }
                mapi_table_set(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void setinstruction(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args_minimum(U, 3);

            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");

            morphine_instruction_t instruction;
            mapi_push_arg(U, 2);
            if (mapi_is_type(U, "table")) {
                maux_expect_args(U, 3);

                mapi_push_string(U, "opcode");
                mapi_table_getoe(U);
                instruction.opcode = maux_opcode_from_name(U, mapi_get_cstr(U));
                mapi_pop(U, 1);

                mapi_push_string(U, "line");
                mapi_table_getoe(U);
                instruction.line = mapi_get_size(U, "line");
                mapi_pop(U, 1);

                size_t count = mapi_opcode(U, instruction.opcode);
                for (size_t i = 0; i < count; i++) {
                    mapi_push_stringf(U, "argument%zu", i);
                    mapi_table_getoe(U);
                    instruction.arguments[i] = (ml_argument) mapi_get_size(U, "argument");
                    mapi_pop(U, 1);
                }
            } else {
                instruction.line = mapi_get_size(U, "line");

                maux_expect_args_minimum(U, 4);
                mapi_push_arg(U, 3);
                instruction.opcode = maux_opcode_from_name(U, mapi_get_cstr(U));

                ml_size count = mapi_opcode(U, instruction.opcode);
                maux_expect_args(U, 4 + count);
                for (ml_size i = 0; i < count; i++) {
                    mapi_push_arg(U, i + 4);
                    instruction.arguments[i] = (ml_argument) mapi_get_size(U, "argument");
                }
            }

            mapi_push_arg(U, 0);
            mapi_instruction_set(U, index, instruction);
            maux_nb_leave();
    maux_nb_end
}

static void mutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool value = true;
            if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_FUNCTION);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_FUNCTION);
            }

            mapi_function_mode_mutable(U, value);
        maux_nb_return();
    maux_nb_end
}

static void immutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_FUNCTION);

            mapi_function_mode_mutable(U, false);
        maux_nb_return();
    maux_nb_end
}

static void accessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool value = true;
            if (mapi_args(U) == 1) {
                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_FUNCTION);
            } else {
                maux_expect_args(U, 2);

                mapi_push_arg(U, 1);
                maux_expect(U, MTYPE_BOOLEAN);
                value = mapi_get_boolean(U);
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_FUNCTION);
            }

            mapi_function_mode_accessible(U, value);
        maux_nb_return();
    maux_nb_end
}

static void inaccessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_FUNCTION);

            mapi_function_mode_accessible(U, false);
        maux_nb_return();
    maux_nb_end
}

static void lockmode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_FUNCTION);

            mapi_function_lock_mode(U);
        maux_nb_return();
    maux_nb_end
}

static void ismutable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_FUNCTION);

            bool value = mapi_function_mode_is_mutable(U);
            mapi_push_boolean(U, value);
        maux_nb_return();
    maux_nb_end
}

static void isaccessible(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_FUNCTION);

            bool value = mapi_function_mode_is_accessible(U);
            mapi_push_boolean(U, value);
        maux_nb_return();
    maux_nb_end
}

static void modeislocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_FUNCTION);

            bool value = mapi_function_mode_is_locked(U);
            mapi_push_boolean(U, value);
        maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("create", create),
    MAUX_CONSTRUCT_FUNCTION("copy", copy),
    MAUX_CONSTRUCT_FUNCTION("info", info),
    MAUX_CONSTRUCT_FUNCTION("getinstruction", getinstruction),
    MAUX_CONSTRUCT_FUNCTION("setinstruction", setinstruction),
    MAUX_CONSTRUCT_FUNCTION("getconstant", getconstant),
    MAUX_CONSTRUCT_FUNCTION("setconstant", setconstant),
    MAUX_CONSTRUCT_FUNCTION("mutable", mutable),
    MAUX_CONSTRUCT_FUNCTION("immutable", immutable),
    MAUX_CONSTRUCT_FUNCTION("accessible", accessible),
    MAUX_CONSTRUCT_FUNCTION("inaccessible", inaccessible),
    MAUX_CONSTRUCT_FUNCTION("lockmode", lockmode),
    MAUX_CONSTRUCT_FUNCTION("ismutable", ismutable),
    MAUX_CONSTRUCT_FUNCTION("isaccessible", isaccessible),
    MAUX_CONSTRUCT_FUNCTION("modeislocked", modeislocked),

#define mspec_instruction_opcode(n, s)            MAUX_CONSTRUCT_STRING("opcode."#s, #n),
#define mspec_instruction_args0(n, s)             mspec_instruction_opcode(n, s)
#define mspec_instruction_args1(n, s, a1)         mspec_instruction_opcode(n, s)
#define mspec_instruction_args2(n, s, a1, a2)     mspec_instruction_opcode(n, s)
#define mspec_instruction_args3(n, s, a1, a2, a3) mspec_instruction_opcode(n, s)

#include "morphine/misc/instruction/specification.h"

#undef mspec_instruction_opcode
#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3

    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_function(void) {
    return (morphine_library_t) {
        .name = "function",
        .sharedkey = NULL,
        .init = library_init
    };
}
