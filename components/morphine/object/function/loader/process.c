//
// Created by why-iskra on 31.03.2024.
//

#include "process.h"
#include "morphine/core/throw.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/userdata.h"

struct process_state {
    bool inited;
    morphine_read_t read;
    morphine_finish_t finish;
    void *data;
};

static void process_userdata_free(morphine_instance_t I, void *p) {
    struct process_state *state = p;

    if (state->inited && state->finish != NULL) {
        state->finish(I, state->data);
    }
}

static struct process_state *process_userdata(
    morphine_coroutine_t U,
    morphine_init_t init,
    morphine_read_t read,
    morphine_finish_t finish,
    void *args
) {
    struct userdata *userdata = userdataI_create(U->I, "loader", sizeof(struct process_state));
    struct process_state *state = userdata->data;

    state->inited = false;
    state->read = read;
    state->finish = finish;

    userdataI_set_free(U->I, userdata, process_userdata_free);
    stackI_push(U, valueI_object(userdata));

    if (init == NULL) {
        state->data = args;
    } else {
        state->data = init(U->I, args);
    }

    state->inited = true;

    return state;
}

struct function *process(
    morphine_coroutine_t U,
    morphine_init_t init,
    morphine_read_t read,
    morphine_finish_t finish,
    void *args,
    function_loader_t loader
) {
    struct process_state *state = process_userdata(U, init, read, finish, args);

    size_t stack_size = stackI_space(U);
    struct function *result = loader(U, state);
    stackI_pop(U, stackI_space(U) - stack_size);

    return result;
}

uint8_t process_byte(morphine_coroutine_t U, struct process_state *process_state) {
    const char *error = NULL;
    uint8_t c = process_state->read(U->I, process_state->data, &error);

    if (error != NULL) {
        throwI_error(U->I, error);
    }

    return c;
}
