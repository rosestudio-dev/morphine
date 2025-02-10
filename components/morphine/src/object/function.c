//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/function.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/core/instruction.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include <string.h>

struct function *functionI_create(
    morphine_instance_t I,
    struct string *name,
    ml_line line,
    ml_size instructions_count,
    ml_size constants_count,
    ml_size slots_count,
    ml_size params_count
) {
    if (name == NULL) {
        throwI_error(I, "function name is null");
    }

    if (instructions_count > mm_typemax(ml_argument)) {
        throwI_error(I, "too many instructions");
    }

    if (constants_count > mm_typemax(ml_argument)) {
        throwI_error(I, "too many constants");
    }

    if (slots_count > mm_typemax(ml_argument)) {
        throwI_error(I, "too many slots");
    }

    if (params_count > mm_typemax(ml_argument)) {
        throwI_error(I, "too many params");
    }

    ml_size stack_size = mm_overflow_opc_add(slots_count, params_count, throwI_error(I, "too many slots and params"));

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(name));

    // create
    struct function *result = allocI_uni(I, NULL, sizeof(struct function));
    (*result) = (struct function) {
        .name = name,
        .line = line,
        .instructions_count = 0,
        .constants_count = 0,
        .slots_count = slots_count,
        .params_count = params_count,
        .constants = NULL,
        .instructions = NULL,
        .stack_size = stack_size
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_FUNCTION);

    // config
    gcI_safe(I, valueI_object(result));

    result->instructions = allocI_vec(I, NULL, instructions_count, sizeof(morphine_instruction_t));
    result->instructions_count = instructions_count;
    for (ml_size i = 0; i < instructions_count; i++) {
        result->instructions[i] = (morphine_instruction_t) {
            .line = 0,
            .opcode = MTYPE_OPCODE_NOP,
            .argument1 = 0,
            .argument2 = 0,
            .argument3 = 0,
        };
    }

    result->constants = allocI_vec(I, NULL, constants_count, sizeof(struct value));
    result->constants_count = constants_count;
    for (ml_size i = 0; i < constants_count; i++) {
        result->constants[i] = valueI_nil;
    }

    gcI_safe_exit(I);

    return result;
}

void functionI_free(morphine_instance_t I, struct function *function) {
    allocI_free(I, function->instructions);
    allocI_free(I, function->constants);
    allocI_free(I, function);
}

morphine_instruction_t functionI_instruction_get(morphine_instance_t I, struct function *function, ml_size index) {
    if (index >= function->instructions_count) {
        throwI_error(I, "instruction index was out of bounce");
    }

    return function->instructions[index];
}

void functionI_instruction_set(
    morphine_instance_t I,
    struct function *function,
    ml_size index,
    morphine_instruction_t instruction
) {
    if (index >= function->instructions_count) {
        throwI_error(I, "instruction index was out of bounce");
    }

    for (ml_size i = instructionI_opcode_args(instruction.opcode, NULL); i < MORPHINE_INSTRUCTION_ARGS_COUNT; i++) {
        instruction.arguments[i] = 0;
    }

    bool is_valid = instructionI_validate(instruction, function);

    if (!is_valid) {
        throwI_error(I, "instruction structure corrupted");
    }

    function->instructions[index] = instruction;
}

struct value functionI_constant_get(morphine_instance_t I, struct function *function, ml_size index) {
    if (index >= function->constants_count) {
        throwI_error(I, "constant index was out of bounce");
    }

    return function->constants[index];
}

void functionI_constant_set(morphine_instance_t I, struct function *function, ml_size index, struct value value) {
    if (index >= function->constants_count) {
        throwI_error(I, "constant index was out of bounce");
    }

    bool supported = valueI_is_nil(value) || valueI_is_integer(value) || valueI_is_decimal(value)
                     || valueI_is_boolean(value) || valueI_is_string(value) || valueI_is_function(value);

    if (!supported) {
        throwI_error(I, "unsupported constant type");
    }

    function->constants[index] = gcI_valbarrier(I, function, value);
}

struct function *functionI_copy(morphine_instance_t I, struct function *function) {
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(function));
    struct function *result = functionI_create(
        I,
        function->name,
        function->line,
        function->instructions_count,
        function->constants_count,
        function->slots_count,
        function->params_count
    );
    gcI_safe(I, valueI_object(result));

    for (ml_size i = 0; i < function->instructions_count; i++) {
        morphine_instruction_t instruction = functionI_instruction_get(I, function, i);
        functionI_instruction_set(I, result, i, instruction);
    }

    for (ml_size i = 0; i < function->constants_count; i++) {
        struct value value = functionI_constant_get(I, function, i);
        functionI_constant_set(I, result, i, value);
    }

    gcI_safe_exit(I);

    return result;
}
