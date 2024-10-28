//
// Created by why-iskra on 06.10.2024.
//

#include "instructions.h"

struct instructions {
    ml_size size;
    struct instruction *array;
};

struct instructions *instructions_alloc(morphine_coroutine_t U) {
    struct instructions *instructions = mapi_allocator_uni(
        mapi_instance(U),
        NULL,
        sizeof(struct instructions)
    );

    (*instructions) = (struct instructions) {
        .size = 0,
        .array = NULL
    };

    return instructions;
}

void instructions_free(morphine_instance_t I, struct instructions *instructions) {
    if (instructions != NULL) {
        mapi_allocator_free(I, instructions->array);
        mapi_allocator_free(I, instructions);
    }
}

void instructions_build(morphine_coroutine_t U, struct instructions *instructions) {
    ml_size size = mapi_instruction_size(U);

    instructions->array = mapi_allocator_vec(
        mapi_instance(U),
        instructions->array,
        size,
        sizeof(struct instruction)
    );

    instructions->size = size;

    for (ml_size i = 0; i < size; i++) {
        morphine_instruction_t instruction = mapi_instruction_get(U, i);
        instructions->array[i] = instruction_convert(U, instruction);
    }
}

ml_size instructions_size(struct instructions *instructions) {
    return instructions->size;
}

struct instruction instructions_get(
    morphine_coroutine_t U,
    struct instructions *instructions,
    ml_size index
) {
    if (index >= instructions->size) {
        mapi_error(U, "instruction index out of bounce");
    }

    return instructions->array[index];
}
