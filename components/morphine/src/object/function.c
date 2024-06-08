//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/function.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"
#include "morphine/instruction/info.h"
#include <string.h>

struct function *functionI_create(
    morphine_instance_t I,
    ml_size name_len,
    ml_size constants_count,
    ml_size instructions_count,
    ml_size statics_count,
    ml_size arguments_count,
    ml_size slots_count,
    ml_size params_count
) {
    if (name_len > MLIMIT_FUNCTION_NAME) {
        throwI_error(I, "Function name too big");
    }

    struct function *result = allocI_uni(I, NULL, sizeof(struct function));

    (*result) = (struct function) {
        .complete = false,
        .name = NULL,
        .name_len = 0,
        .constants_count = 0,
        .instructions_count = 0,
        .statics_count = 0,
        .arguments_count = arguments_count,
        .slots_count = slots_count,
        .params_count = params_count,
        .constants = NULL,
        .instructions = NULL,
        .statics = NULL,
        .registry_key = valueI_nil
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_FUNCTION);

    size_t rollback = gcI_safe_obj(I, objectI_cast(result));

    result->name = allocI_vec(I, NULL, name_len + 1, sizeof(char));
    result->name_len = name_len;
    memset(result->name, 0, (name_len + 1) * sizeof(char));

    result->instructions = allocI_vec(I, NULL, instructions_count, sizeof(morphine_instruction_t));
    result->instructions_count = instructions_count;
    for (size_t i = 0; i < instructions_count; i++) {
        result->instructions[i] = (morphine_instruction_t) {
            .line = 0,
            .opcode = MORPHINE_OPCODE_YIELD,
            .argument1 = 0,
            .argument2 = 0,
            .argument3 = 0
        };
    }

    result->constants = allocI_vec(I, NULL, constants_count, sizeof(struct value));
    result->constants_count = constants_count;
    for (size_t i = 0; i < constants_count; i++) {
        result->constants[i] = valueI_nil;
    }

    result->statics = allocI_vec(I, NULL, statics_count, sizeof(struct value));
    result->statics_count = statics_count;
    for (size_t i = 0; i < statics_count; i++) {
        result->statics[i] = valueI_nil;
    }

    gcI_reset_safe(I, rollback);

    return result;
}

void functionI_free(morphine_instance_t I, struct function *function) {
    allocI_free(I, function->name);
    allocI_free(I, function->instructions);
    allocI_free(I, function->constants);
    allocI_free(I, function->statics);
    allocI_free(I, function);
}

void functionI_complete(morphine_instance_t I, struct function *function) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    function->complete = true;
}

morphine_instruction_t functionI_instruction_get(
    morphine_instance_t I,
    struct function *function,
    ml_size index
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->instructions_count) {
        throwI_error(I, "Instruction index was out of bounce");
    }

    return function->instructions[index];
}

void functionI_instruction_set(
    morphine_instance_t I,
    struct function *function,
    ml_size index,
    morphine_instruction_t instruction
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (function->complete) {
        throwI_error(I, "Function is complete");
    }

    if (index >= function->instructions_count) {
        throwI_error(I, "Instruction index was out of bounce");
    }

    bool is_valid = instructionI_validate(
        instruction,
        function->arguments_count,
        function->slots_count,
        function->params_count,
        function->constants_count
    );

    if (!is_valid) {
        throwI_error(I, "Instruction structure corrupted");
    }

    function->instructions[index] = instruction;
}

ml_line functionI_line_get(
    morphine_instance_t I,
    struct function *function,
    ml_size index
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->instructions_count) {
        throwI_error(I, "Instruction index was out of bounce");
    }

    return function->instructions[index].line;
}

void functionI_line_set(
    morphine_instance_t I,
    struct function *function,
    ml_size index,
    ml_line line
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (function->complete) {
        throwI_error(I, "Function is complete");
    }

    if (index >= function->instructions_count) {
        throwI_error(I, "Instruction index was out of bounce");
    }

    function->instructions[index].line = line;
}

struct value functionI_constant_get(
    morphine_instance_t I,
    struct function *function,
    ml_size index
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->constants_count) {
        throwI_error(I, "Constant index was out of bounce");
    }

    return function->constants[index];
}

void functionI_constant_set(
    morphine_instance_t I,
    struct function *function,
    ml_size index,
    struct value value
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (function->complete) {
        throwI_error(I, "Function is complete");
    }

    if (index >= function->constants_count) {
        throwI_error(I, "Constant index was out of bounce");
    }

    gcI_barrier(I, function, value);
    function->constants[index] = value;
}

struct value functionI_static_get(
    morphine_instance_t I,
    struct function *function,
    ml_size index
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->statics_count) {
        throwI_error(I, "Static index was out of bounce");
    }

    return function->statics[index];
}

void functionI_static_set(
    morphine_instance_t I,
    struct function *function,
    ml_size index,
    struct value value
) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->statics_count) {
        throwI_error(I, "Static index was out of bounce");
    }

    gcI_barrier(I, function, value);
    function->statics[index] = value;
}
