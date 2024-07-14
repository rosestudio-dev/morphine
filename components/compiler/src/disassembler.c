//
// Created by why-iskra on 10.06.2024.
//

#include <string.h>
#include "morphinec/disassembler.h"

#define printn(str, s) do { mapi_peek(U, 1); mapi_sio_write(U, (const uint8_t *) (str), (s)); mapi_pop(U, 1); } while(false)
#define printf(args...) do { mapi_peek(U, 1); mapi_sio_printf(U, args); mapi_pop(U, 1); } while(false)

#define opcode_case(n) case MORPHINE_OPCODE_##n: return #n;

static const char *opcode2str(morphine_opcode_t opcode) {
    switch (opcode) {
        opcode_case(YIELD)
        opcode_case(LOAD)
        opcode_case(MOVE)
        opcode_case(PARAM)
        opcode_case(ARG)
        opcode_case(ENV)
        opcode_case(SELF)
        opcode_case(RECURSION)
        opcode_case(VECTOR)
        opcode_case(TABLE)
        opcode_case(GET)
        opcode_case(SET)
        opcode_case(ITERATOR)
        opcode_case(ITERATOR_INIT)
        opcode_case(ITERATOR_HAS)
        opcode_case(ITERATOR_NEXT)
        opcode_case(JUMP)
        opcode_case(JUMP_IF)
        opcode_case(GET_STATIC)
        opcode_case(SET_STATIC)
        opcode_case(GET_CLOSURE)
        opcode_case(SET_CLOSURE)
        opcode_case(CLOSURE)
        opcode_case(CALL)
        opcode_case(LEAVE)
        opcode_case(RESULT)
        opcode_case(ADD)
        opcode_case(SUB)
        opcode_case(MUL)
        opcode_case(DIV)
        opcode_case(MOD)
        opcode_case(EQUAL)
        opcode_case(LESS)
        opcode_case(AND)
        opcode_case(OR)
        opcode_case(CONCAT)
        opcode_case(TYPE)
        opcode_case(NEGATIVE)
        opcode_case(NOT)
        opcode_case(REF)
        opcode_case(DEREF)
        opcode_case(LENGTH)
    }

    return "?";
}

static void print_description(morphine_coroutine_t U, morphine_instruction_t instr) {
#define APR "%"MLIMIT_ARGUMENT_PR
#define SLOT "slot "APR
#define arg(n) instr.argument##n
    switch (instr.opcode) {
        case MORPHINE_OPCODE_YIELD:
            printf("yield");
            return;
        case MORPHINE_OPCODE_LOAD:
            printf(SLOT" = constant "APR, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_MOVE:
            printf(SLOT" = "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_PARAM:
            printf("param "APR" = "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_ARG:
            printf(SLOT" = arg "APR, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_ENV:
            printf(SLOT" = env", arg(1));
            return;
        case MORPHINE_OPCODE_SELF:
            printf(SLOT" = self", arg(1));
            return;
        case MORPHINE_OPCODE_RECURSION:
            printf(SLOT" = callable", arg(1));
            return;
        case MORPHINE_OPCODE_VECTOR:
            printf(SLOT" = vector with size "APR, arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_TABLE:
            printf(SLOT" = table", arg(1));
            return;
        case MORPHINE_OPCODE_GET:
            printf(SLOT" = ("SLOT")["SLOT"]", arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_SET:
            printf("("SLOT")["SLOT"] = "SLOT, arg(1), arg(2), arg(3));
            return;
        case MORPHINE_OPCODE_ITERATOR:
            printf(SLOT" = iterator from "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_ITERATOR_INIT:
            printf("init iterator in "SLOT, arg(1));
            return;
        case MORPHINE_OPCODE_ITERATOR_HAS:
            printf(SLOT" = iterator has in "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_ITERATOR_NEXT:
            printf(SLOT" = iterator next in "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_JUMP:
            printf("jump to "APR, arg(1));
            return;
        case MORPHINE_OPCODE_JUMP_IF:
            printf("jump if "SLOT" to "APR" else "APR, arg(1), arg(2), arg(3));
            return;
        case MORPHINE_OPCODE_GET_STATIC:
            printf(SLOT" = static "APR" from "SLOT, arg(3), arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_SET_STATIC:
            printf("static "APR" from "SLOT" = "SLOT, arg(2), arg(1), arg(3));
            return;
        case MORPHINE_OPCODE_GET_CLOSURE:
            printf(SLOT" = closure "APR" from "SLOT, arg(3), arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_SET_CLOSURE:
            printf("closure "APR" from "SLOT" = "SLOT, arg(2), arg(1), arg(3));
            return;
        case MORPHINE_OPCODE_CLOSURE:
            printf(SLOT" = closure from "SLOT" with size "APR, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_CALL:
            printf("call(self "SLOT") "SLOT" with "APR" args", arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_LEAVE:
            printf("leave");
            return;
        case MORPHINE_OPCODE_RESULT:
            printf(SLOT" = call result", arg(1));
            return;
        case MORPHINE_OPCODE_ADD:
            printf(SLOT" = "SLOT" + "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_SUB:
            printf(SLOT" = "SLOT" - "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_MUL:
            printf(SLOT" = "SLOT" * "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_DIV:
            printf(SLOT" = "SLOT" / "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_MOD:
            printf(SLOT" = "SLOT" %% "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_EQUAL:
            printf(SLOT" = "SLOT" == "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_LESS:
            printf(SLOT" = "SLOT" < "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_AND:
            printf(SLOT" = "SLOT" and "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_OR:
            printf(SLOT" = "SLOT" or "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_CONCAT:
            printf(SLOT" = "SLOT" .. "SLOT, arg(3), arg(1), arg(2));
            return;
        case MORPHINE_OPCODE_TYPE:
            printf(SLOT" = type "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_NEGATIVE:
            printf(SLOT" = - "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_NOT:
            printf(SLOT" = not "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_REF:
            printf(SLOT" = ref "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_DEREF:
            printf(SLOT" = deref "SLOT, arg(2), arg(1));
            return;
        case MORPHINE_OPCODE_LENGTH:
            printf(SLOT" = len "SLOT, arg(2), arg(1));
            return;
    }
}

static inline size_t numlen(size_t num) {
    size_t len = 1;
    while (num > 9) {
        len++;
        num /= 10;
    }
    return len;
}

static inline void spaces(morphine_coroutine_t U, size_t len, size_t count) {
    for (size_t i = len; i < count; i++) {
        printn(" ", 1);
    }
}

static void print_instructions(morphine_coroutine_t U) {
    size_t name_len = 0;
    size_t max_line = 0;
    size_t max_arg1 = 0;
    size_t max_arg2 = 0;
    size_t max_arg3 = 0;

    ml_size count = mapi_instruction_size(U);
    for (ml_size i = 0; i < count; i++) {
        morphine_instruction_t instruction = mapi_instruction_get(U, i);

        size_t len = strlen(opcode2str(instruction.opcode));
        if (name_len < len) {
            name_len = len;
        }

        if (max_line < instruction.line) {
            max_line = instruction.line;
        }

        if (max_arg1 < instruction.argument1) {
            max_arg1 = instruction.argument1;
        }

        if (max_arg2 < instruction.argument2) {
            max_arg2 = instruction.argument2;
        }

        if (max_arg3 < instruction.argument3) {
            max_arg3 = instruction.argument3;
        }
    }

    size_t index_len = numlen(count - 1);
    size_t line_len = numlen(max_line);
    size_t arg1_len = numlen(max_arg1);
    size_t arg2_len = numlen(max_arg2);
    size_t arg3_len = numlen(max_arg3);

    printf("instructions (%"MLIMIT_SIZE_PR"):\n", count);
    for (ml_size i = 0; i < count; i++) {
        morphine_instruction_t instruction = mapi_instruction_get(U, i);

        printf("    %"MLIMIT_SIZE_PR". ", i);
        spaces(U, numlen(i), index_len);

        printf("[line: %"MLIMIT_LINE_PR"] ", instruction.line);
        spaces(U, numlen(instruction.line), line_len);

        printf("%s    ", opcode2str(instruction.opcode));
        spaces(U, strlen(opcode2str(instruction.opcode)), name_len);

        size_t args = mapi_opcode_args(U, instruction.opcode);

        if (args > 0) {
            printf("%"MLIMIT_ARGUMENT_PR" ", instruction.argument1);
            spaces(U, numlen(instruction.argument1), arg1_len);
        } else {
            printn(" ", 1);
            spaces(U, 0, arg1_len);
        }

        if (args > 1) {
            printf("%"MLIMIT_ARGUMENT_PR" ", instruction.argument2);
            spaces(U, numlen(instruction.argument2), arg2_len);
        } else {
            printn(" ", 1);
            spaces(U, 0, arg2_len);
        }

        if (args > 2) {
            printf("%"MLIMIT_ARGUMENT_PR, instruction.argument3);
            spaces(U, numlen(instruction.argument3), arg3_len);
        } else {
            spaces(U, 0, arg3_len);
        }

        printn("    ; ", 6);

        print_description(U, instruction);

        printn("\n", 1);
    }
}

static void print_constants(morphine_coroutine_t U) {
    size_t type_len = 0;

    ml_size count = mapi_constant_size(U);
    for (ml_size i = 0; i < count; i++) {
        mapi_constant_get(U, i);

        const char *type = mapi_type(U);
        size_t len = strlen(type);
        if (type_len < len) {
            type_len = len;
        }

        mapi_pop(U, 1);
    }

    size_t index_len = numlen(count - 1);

    printf("constants (%"MLIMIT_SIZE_PR"):\n", count);
    for (ml_size i = 0; i < count; i++) {
        printf("    %"MLIMIT_SIZE_PR". ", i);
        spaces(U, numlen(i), index_len);

        mapi_constant_get(U, i);
        const char *type = mapi_type(U);
        mapi_pop(U, 1);

        printf("%s ", type);
        spaces(U, strlen(type), type_len);

        mapi_constant_get(U, i);
        mapi_to_string(U);
        const char *value = mapi_get_string(U);
        mapi_rotate(U, 3);

        printf("%s\n", value);

        mapi_rotate(U, 3);
        mapi_rotate(U, 3);
        mapi_pop(U, 1);
    }
}

MORPHINE_API void mcapi_disassembly(morphine_coroutine_t U) {
    if (!mapi_function_is_complete(U)) {
        printf("incomplete ");
    }

    const char *name = mapi_function_name(U);
    ml_line line = mapi_function_line(U);
    ml_size args = mapi_function_arguments(U);
    ml_size slots = mapi_function_slots(U);
    ml_size params = mapi_function_params(U);
    ml_size statics = mapi_static_size(U);

    printf("function %s (at line: %"MLIMIT_LINE_PR")\n", name, line);
    printf("    args:    %"MLIMIT_SIZE_PR"\n", args);
    printf("    slots:   %"MLIMIT_SIZE_PR"\n", slots);
    printf("    params:  %"MLIMIT_SIZE_PR"\n", params);
    printf("    statics: %"MLIMIT_SIZE_PR"\n", statics);

    print_instructions(U);
    print_constants(U);
}