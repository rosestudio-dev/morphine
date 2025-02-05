//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/function.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/misc/instruction.h"

MORPHINE_API void mapi_push_function(
    morphine_coroutine_t U,
    ml_line line,
    ml_size instructions_count,
    ml_size constants_count,
    ml_size slots_count,
    ml_size params_count
) {
    struct string *name = valueI_as_string_or_error(U->I, stackI_peek(U, 0));

    struct function *function = functionI_create(
        U->I,
        name,
        line,
        instructions_count,
        constants_count,
        slots_count,
        params_count
    );

    stackI_replace(U, 0, valueI_object(function));
}

MORPHINE_API void mapi_function_copy(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, valueI_object(functionI_copy(U->I, function)));
}

MORPHINE_API void mapi_function_name(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, valueI_object(function->name));
}

MORPHINE_API ml_line mapi_function_line(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->line;
}

MORPHINE_API ml_size mapi_function_slots(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->slots_count;
}

MORPHINE_API ml_size mapi_function_params(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->params_count;
}

MORPHINE_API void mapi_function_mode_mutable(morphine_coroutine_t U, bool is_mutable) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    functionI_mode_mutable(U->I, function, is_mutable);
}

MORPHINE_API void mapi_function_mode_accessible(morphine_coroutine_t U, bool is_accessible) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    functionI_mode_accessible(U->I, function, is_accessible);
}

MORPHINE_API void mapi_function_lock_mode(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    functionI_lock_mode(U->I, function);
}

MORPHINE_API bool mapi_function_mode_is_mutable(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->mode.mutable;
}

MORPHINE_API bool mapi_function_mode_is_accessible(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->mode.accessible;
}

MORPHINE_API bool mapi_function_mode_is_locked(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->lock.mode;
}

MORPHINE_API void mapi_constant_get(morphine_coroutine_t U, ml_size index) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    struct value value = functionI_constant_get(U->I, function, index);
    stackI_push(U, value);
}

MORPHINE_API void mapi_constant_set(morphine_coroutine_t U, ml_size index) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 1));
    functionI_constant_set(U->I, function, index, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

MORPHINE_API ml_size mapi_constant_size(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->constants_count;
}

MORPHINE_API morphine_instruction_t mapi_instruction_get(morphine_coroutine_t U, ml_size index) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return functionI_instruction_get(U->I, function, index);
}

MORPHINE_API void mapi_instruction_set(
    morphine_coroutine_t U,
    ml_size index,
    morphine_instruction_t instruction
) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    functionI_instruction_set(U->I, function, index, instruction);
}

MORPHINE_API ml_size mapi_instruction_size(morphine_coroutine_t U) {
    struct function *function = valueI_as_function_or_error(U->I, stackI_peek(U, 0));
    return function->instructions_count;
}

MORPHINE_API ml_size mapi_opcode(morphine_coroutine_t U, mtype_opcode_t opcode) {
    bool valid = false;
    ml_size result = instructionI_opcode_args(opcode, &valid);

    if (!valid) {
        mapi_error(U, "invalid opcode");
    }

    return result;
}
