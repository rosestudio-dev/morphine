//
// Created by why-iskra on 31.03.2024.
//

#include "process.h"
#include "morphine/stack/access.h"
#include "morphine/core/throw.h"

struct process_state {
    morphine_state_t S;
    jmp_buf jump;
    void *data;
    morphine_loader_read_t read;
    const char *message;
};

struct proto *process(
    morphine_state_t S,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args,
    function_loader_t loader
) {
    struct process_state process_state = {
        .S = S,
        .read = read,
    };

    if (init == NULL) {
        process_state.data = args;
    } else {
        process_state.data = init(S, args);
    }

    size_t stack_size = stackI_space_size(S);

    struct proto *result;

    if (setjmp(process_state.jump) != 0) {
        if (finish != NULL) {
            finish(S, process_state.data);
        }

        throwI_message_error(S, process_state.message);
    } else {
        result = loader(S, &process_state);

        stackI_pop(S, stackI_space_size(S) - stack_size);

        if (finish != NULL) {
            finish(S, process_state.data);
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
    uint8_t c = process_state->read(process_state->S, process_state->data, &error);

    if (error != NULL) {
        process_error(process_state, error);
    }

    return c;
}
