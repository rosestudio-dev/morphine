//
// Created by why-iskra on 24.09.2024.
//

#include <stdio.h>
#include "env.h"

morphine_noret void env_signal(morphine_instance_t I, void *data, bool is_panic) {
    struct env *env = data;
    const char *message = mapi_signal_message(I);
    fprintf(stderr, "morphine %s: %s\n", is_panic ? "panic" : "error", message);

    if (I != NULL && !mapi_is_nested_signal(I) && !is_panic) {
        mapi_close(I);
    }

    longjmp(env->abort_jmp, 1);
}

morphine_noret void env_exit(morphine_instance_t I, ml_integer code) {
    struct env *env = mapi_instance_data(I);

    env->exit.I = I;
    env->exit.code = (int) code;

    longjmp(env->exit.jmp, 1);
}
