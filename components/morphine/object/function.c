//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/function.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/allocator.h"
#include <string.h>

bool functionI_uuid_equal(struct uuid a, struct uuid b) {
    return (a.most_significant_bits == b.most_significant_bits) &&
           (a.least_significant_bits == b.least_significant_bits);
}

struct function *functionI_create(
    morphine_instance_t I,
    struct uuid uuid,
    size_t name_len,
    size_t constants_count,
    size_t instructions_count,
    size_t statics_count
) {
    size_t function_size = sizeof(struct function);
    size_t name_size = sizeof(char) * (name_len + 1);
    size_t constants_size = sizeof(struct value) * constants_count;
    size_t instructions_size = sizeof(instruction_t) * instructions_count;
    size_t statics_size = sizeof(struct value) * statics_count;

    size_t size = function_size + name_size + constants_size + instructions_size + statics_size;

    struct function *result = allocI_uni(I, NULL, size);

    void *ptr_name = ((void *) result) + function_size;
    void *ptr_constants = ((void *) ptr_name) + name_size;
    void *ptr_instructions = ((void *) ptr_constants) + constants_size;
    void *ptr_statics = ((void *) ptr_instructions) + instructions_size;

    (*result) = (struct function) {
        .uuid = uuid,
        .name = ptr_name,
        .name_len = name_len,
        .constants_count = constants_count,
        .instructions_count = instructions_count,
        .statics_count = statics_count,
        .arguments_count = 0,
        .slots_count = 0,
        .params_count = 0,
        .constants = ptr_constants,
        .instructions = ptr_instructions,
        .statics = ptr_statics,
        .registry_key = valueI_nil
    };

    for (size_t i = 0; i < constants_count; i++) {
        result->constants[i] = valueI_nil;
    }

    for (size_t i = 0; i < statics_count; i++) {
        result->statics[i] = valueI_nil;
    }

    result->name[name_len] = '\0';

    objectI_init(I, objectI_cast(result), OBJ_TYPE_FUNCTION);

    return result;
}

void functionI_free(morphine_instance_t I, struct function *function) {
    allocI_free(I, function);
}

struct value functionI_static_get(morphine_instance_t I, struct function *function, size_t index) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->statics_count) {
        throwI_error(I, "Static index was out of bounce");
    }

    return function->statics[index];
}

void functionI_static_set(morphine_instance_t I, struct function *function, size_t index, struct value value) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->statics_count) {
        throwI_error(I, "Static index was out of bounce");
    }

    gcI_barrier(function, value);
    function->statics[index] = value;
}


struct value functionI_constant_get(morphine_instance_t I, struct function *function, size_t index) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->constants_count) {
        throwI_error(I, "Constant index was out of bounce");
    }

    return function->constants[index];
}

void functionI_constant_set(morphine_instance_t I, struct function *function, size_t index, struct value value) {
    if (function == NULL) {
        throwI_error(I, "Function is null");
    }

    if (index >= function->constants_count) {
        throwI_error(I, "Constant index was out of bounce");
    }

    gcI_barrier(function, value);
    function->constants[index] = value;
}
