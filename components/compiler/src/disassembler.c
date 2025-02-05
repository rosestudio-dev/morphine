//
// Created by why-iskra on 10.06.2024.
//

#include <string.h>
#include "morphinec/disassembler.h"

#define printn(str, s) do { mapi_peek(U, 1); mapi_stream_write(U, (const uint8_t *) (str), (s)); mapi_pop(U, 1); } while(false)
#define printf(args...) do { mapi_peek(U, 1); mapi_stream_printf(U, args); mapi_pop(U, 1); } while(false)


static const char *opcode2str(mtype_opcode_t opcode) {
    switch (opcode) {
#define opcode_case(n, m)                         case MTYPE_OPCODE_##n: return #m;
#define mspec_instruction_args0(n, s)             opcode_case(n, s)
#define mspec_instruction_args1(n, s, a1)         opcode_case(n, s)
#define mspec_instruction_args2(n, s, a1, a2)     opcode_case(n, s)
#define mspec_instruction_args3(n, s, a1, a2, a3) opcode_case(n, s)

#include "morphine/misc/instruction/specification.h"

#undef opcode_case
#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
    }

    return "?";
}

static void print_description(morphine_coroutine_t U, morphine_instruction_t instr) {
#define APR "%"MLIMIT_ARGUMENT_PR
#define SLOT "slot "APR
#define arg(n) instr.argument##n
    switch (instr.opcode) {
        case MTYPE_OPCODE_NO_OPERATION:
            printf("no operation");
            return;
        case MTYPE_OPCODE_YIELD:
            printf("yield");
            return;
        case MTYPE_OPCODE_LOAD:
            printf(SLOT" = constant "APR, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_MOVE:
            printf(SLOT" = "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_PARAM:
            printf("param "APR" = "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_ARG:
            printf(SLOT" = arg "APR, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_ENV:
            printf(SLOT" = env", arg(1));
            return;
        case MTYPE_OPCODE_INVOKED:
            printf(SLOT" = invoked callable", arg(1));
            return;
        case MTYPE_OPCODE_VECTOR:
            printf(SLOT" = vector with size "APR, arg(1), arg(2));
            return;
        case MTYPE_OPCODE_TABLE:
            printf(SLOT" = table", arg(1));
            return;
        case MTYPE_OPCODE_GET:
            printf(SLOT" = ("SLOT")["SLOT"]", arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_SET:
            printf("("SLOT")["SLOT"] = "SLOT, arg(1), arg(2), arg(3));
            return;
        case MTYPE_OPCODE_ITERATOR:
            printf(SLOT" = iterator from "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_ITERATOR_INIT:
            printf("init iterator in "SLOT" with name "SLOT" for key and "SLOT" for value", arg(1), arg(2), arg(3));
            return;
        case MTYPE_OPCODE_ITERATOR_HAS:
            printf(SLOT" = iterator has in "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_ITERATOR_NEXT:
            printf(SLOT" = iterator next in "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_JUMP:
            printf("jump to "APR, arg(1));
            return;
        case MTYPE_OPCODE_JUMP_IF:
            printf("jump if "SLOT" to "APR" else "APR, arg(1), arg(2), arg(3));
            return;
        case MTYPE_OPCODE_CLOSURE:
            printf(SLOT" = create closure for "SLOT" with "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_CLOSURE_VALUE:
            printf(SLOT" = get value from closure "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_CALL:
            printf(SLOT" = call "SLOT" with "APR" args", arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_RETURN:
            printf("return "SLOT, arg(1));
            return;
        case MTYPE_OPCODE_ADD:
            printf(SLOT" = "SLOT" + "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_SUB:
            printf(SLOT" = "SLOT" - "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_MUL:
            printf(SLOT" = "SLOT" * "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_DIV:
            printf(SLOT" = "SLOT" / "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_MOD:
            printf(SLOT" = "SLOT" %% "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_EQUAL:
            printf(SLOT" = "SLOT" == "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_LESS:
            printf(SLOT" = "SLOT" < "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_AND:
            printf(SLOT" = "SLOT" and "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_OR:
            printf(SLOT" = "SLOT" or "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_CONCAT:
            printf(SLOT" = "SLOT" .. "SLOT, arg(3), arg(1), arg(2));
            return;
        case MTYPE_OPCODE_TYPE:
            printf(SLOT" = type "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_NEGATIVE:
            printf(SLOT" = - "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_NOT:
            printf(SLOT" = not "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_REF:
            printf(SLOT" = ref "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_DEREF:
            printf(SLOT" = deref "SLOT, arg(2), arg(1));
            return;
        case MTYPE_OPCODE_LENGTH:
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
    size_t max_args[MORPHINE_INSTRUCTION_ARGS_COUNT];
    for (size_t i = 0; i < MORPHINE_INSTRUCTION_ARGS_COUNT; i++) {
        max_args[i] = 0;
    }

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

        for (size_t a = 0; a < MORPHINE_INSTRUCTION_ARGS_COUNT; a++) {
            if (max_args[a] < instruction.arguments[a]) {
                max_args[a] = instruction.arguments[a];
            }
        }
    }

    size_t index_len = numlen(count - 1);
    size_t line_len = numlen(max_line);
    size_t args_lens[MORPHINE_INSTRUCTION_ARGS_COUNT];
    for (size_t i = 0; i < MORPHINE_INSTRUCTION_ARGS_COUNT; i++) {
        args_lens[i] = numlen(max_args[i]);
    }

    printf("    instructions (%"MLIMIT_SIZE_PR"):\n", count);
    for (ml_size i = 0; i < count; i++) {
        morphine_instruction_t instruction = mapi_instruction_get(U, i);

        printf("        %"MLIMIT_SIZE_PR". ", i);
        spaces(U, numlen(i), index_len);

        printf("[line: %"MLIMIT_LINE_PR"] ", instruction.line);
        spaces(U, numlen(instruction.line), line_len);

        printf("%s    ", opcode2str(instruction.opcode));
        spaces(U, strlen(opcode2str(instruction.opcode)), name_len);

        size_t args = mapi_opcode(U, instruction.opcode);
        for (size_t a = 0; a < MORPHINE_INSTRUCTION_ARGS_COUNT; a++) {
            if (a < args) {
                printf("%"MLIMIT_ARGUMENT_PR" ", instruction.arguments[a]);
                spaces(U, numlen(instruction.arguments[a]), args_lens[a]);
            } else {
                printn(" ", 1);
                spaces(U, 0, args_lens[a]);
            }
        }

        printn("   ; ", 5);

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

    printf("    constants (%"MLIMIT_SIZE_PR"):\n", count);
    for (ml_size i = 0; i < count; i++) {
        printf("        %"MLIMIT_SIZE_PR". ", i);
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
    mapi_function_name(U);
    const char *name = mapi_get_cstr(U);
    mapi_pop(U, 1);

    ml_line line = mapi_function_line(U);
    ml_size slots = mapi_function_slots(U);
    ml_size params = mapi_function_params(U);

    printf("function(%s) {\n", name);
    printf("    line:   %"MLIMIT_LINE_PR"\n", line);
    printf("    slots:  %"MLIMIT_SIZE_PR"\n", slots);
    printf("    params: %"MLIMIT_SIZE_PR"\n", params);
    print_instructions(U);
    print_constants(U);
    printf("}\n");

    mapi_pop(U, 2);
}