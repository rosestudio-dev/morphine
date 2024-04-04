//
// Created by why-iskra on 31.03.2024.
//

#include "process.h"
#include "morphine/stack/access.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"

struct process_state {
    morphine_coroutine_t U;
    jmp_buf jump;
    void *data;
    morphine_loader_read_t read;
    const char *message;
};

struct function *process(
    morphine_coroutine_t U,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args,
    function_loader_t loader
) {
    struct process_state process_state = {
        .U = U,
        .read = read,
    };

    if (init == NULL) {
        process_state.data = args;
    } else {
        process_state.data = init(U, args);
    }

    size_t stack_size = stackI_space_size(U);

    struct function *result;

    if (setjmp(process_state.jump) != 0) {
        if (finish != NULL) {
            finish(U, process_state.data);
        }

        throwI_error(U->I, process_state.message);
    } else {
        result = loader(U, &process_state);

        stackI_pop(U, stackI_space_size(U) - stack_size);

        if (finish != NULL) {
            finish(U, process_state.data);
        }
    }

    return result;
}

morphine_noret void process_error(
    struct process_state *process_state,
    const char *message
) {
    process_state->message = message;
    longjmp(process_state->jump, 1);
}

uint8_t process_byte(struct process_state *process_state) {
    const char *error = NULL;
    uint8_t c = process_state->read(process_state->U, process_state->data, &error);

    if (error != NULL) {
        process_error(process_state, error);
    }

    return c;
}
