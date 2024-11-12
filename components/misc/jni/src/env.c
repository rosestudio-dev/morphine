//
// Created by why-iskra on 24.09.2024.
//

#include <stdio.h>
#include "env.h"

morphine_noret void env_signal(morphine_instance_t I) {
    struct env *env = mapi_instance_data(I);
    const char *message = mapi_signal_message(I);
    fprintf(stderr, "morphine panic: %s\n", message);

    if (I != NULL && !mapi_is_nested_signal(I)) {
        mapi_close(I);
    }

    env->exit.I = NULL;
    longjmp(env->exit.jmp, 1);
}

morphine_noret void env_exit(morphine_instance_t I) {
    struct env *env = mapi_instance_data(I);

    env->exit.I = I;
    longjmp(env->exit.jmp, 1);
}
