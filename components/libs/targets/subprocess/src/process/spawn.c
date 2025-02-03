//
// Created by why on 1/14/25.
//

#include "morphinel/process/spawn.h"
#include "morphine/utils/overflow.h"
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

struct process {
    pid_t pid;

    struct {
        bool has;
        int value;
    } status;

    struct {
        size_t count;
        char **array;
    } env;
};

struct pipe_data {
    int input;
    int output;
    bool input_closed;
    bool output_closed;
};

// pipe

struct pipe_args {
    struct pipe_data *data;
};

static void pipe_open(morphine_instance_t I, void *data, void *args) {
    struct pipe_data *pipe_data = data;
    struct pipe_args *pipe_args = args;

    int socket[2];
    if (pipe(socket) != 0) {
        mapi_ierror(I, "unable to create pipe");
    }

    fcntl(socket[0], F_SETFL, fcntl(socket[0], F_GETFL) | O_NONBLOCK);
    fcntl(socket[1], F_SETFL, fcntl(socket[1], F_GETFL) | O_NONBLOCK);

    (*pipe_data) = (struct pipe_data) {
        .input = socket[0],
        .output = socket[1],
        .input_closed = false,
        .output_closed = false,
    };

    pipe_args->data = pipe_data;
}

static void pipe_close(morphine_instance_t I, void *data) {
    (void) I;
    struct pipe_data *pipe_data = data;

    if (!pipe_data->input_closed) {
        close(pipe_data->input);
        pipe_data->input_closed = true;
    }

    if (!pipe_data->output_closed) {
        close(pipe_data->output);
        pipe_data->output_closed = true;
    }
}

static size_t pipe_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    (void) I;
    struct pipe_data *pipe_data = data;

    if (pipe_data->input_closed) {
        mapi_ierror(I, "pipe input was closed");
    }

    memset(buffer, 0, size);
    ssize_t result = read(pipe_data->input, buffer, size);
    return result < 0 ? 0 : (size_t) result;
}

static size_t pipe_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    (void) I;
    struct pipe_data *pipe_data = data;

    if (pipe_data->output_closed) {
        mapi_ierror(I, "pipe output was closed");
    }

    ssize_t result = write(pipe_data->output, buffer, size);
    return result < 0 ? 0 : (size_t) result;
}

static struct pipe_data *create_pipe(morphine_coroutine_t U, bool read) {
    morphine_stream_interface_t interface = {
        .data_size = sizeof(struct pipe_data),
        .open = pipe_open,
        .close = pipe_close,
        .read = read ? pipe_read : NULL,
        .write = read ? NULL : pipe_write,
        .flush = NULL,
        .seek = NULL,
        .tell = NULL,
        .eos = NULL,
    };

    struct pipe_args args = { .data = NULL };
    mapi_push_stream(U, interface, false, &args);

    return args.data;
}

// process

static void process_constructor(morphine_instance_t I, void *data) {
    (void) I;
    struct process *process = data;
    (*process) = (struct process) {
        .pid = 0,
        .status.has = false,
        .status.value = 0,
        .env.count = 0,
        .env.array = NULL,
    };
}

static void process_destructor(morphine_instance_t I, void *data) {
    (void) I;
    struct process *process = data;

    for (size_t i = 0; i < process->env.count; i++) {
        mapi_allocator_free(I, process->env.array[i]);
    }

    mapi_allocator_free(I, process->env.array);
}

static void process_type(morphine_coroutine_t U) {
    mapi_type_declare(
        mapi_instance(U),
        MLIB_PROCESS_USERDATA_TYPE,
        sizeof(struct process),
        false,
        process_constructor,
        process_destructor,
        NULL,
        NULL
    );
}

// api

MORPHINE_API void mlapi_process_spawn(morphine_coroutine_t U, bool env) {
    process_type(U);

    struct process *process = mapi_push_userdata(U, MLIB_PROCESS_USERDATA_TYPE);
    if (env) {
        mapi_rotate(U, 2);

        ml_size len = mapi_table_len(U);

        {
            size_t size = len;
            mm_overflow_add(size, 1) {
                mapi_error(U, "env array overflow");
            }

            process->env.array = mapi_allocator_vec(mapi_instance(U), NULL, size + 1, sizeof(char *));
        }

        for (ml_size i = 0; i < len; i++) {
            mapi_table_idx_key(U, i);
            const char *key_str = mapi_get_string(U);
            size_t key_len = mapi_string_len(U);
            mapi_rotate(U, 2);

            mapi_table_idx_get(U, i);
            const char *val_str = mapi_get_string(U);
            size_t val_len = mapi_string_len(U);
            mapi_rotate(U, 2);
            mapi_rotate(U, 3);

            size_t str_size = mm_overflow_opc_add(key_len, val_len, mapi_error(U, "env element overflow"));
            str_size = mm_overflow_opc_add(str_size, 2, mapi_error(U, "env element overflow"));

            char *elem = mapi_allocator_vec(mapi_instance(U), NULL, str_size, sizeof(char));
            memcpy(elem, key_str, key_len);
            memcpy(elem + key_len + 1, val_str, val_len);
            elem[key_len] = '=';
            elem[key_len + val_len + 1] = '\0';

            process->env.array[process->env.count] = elem;
            process->env.count++;

            mapi_pop(U, 2);
        }

        process->env.array[process->env.count] = NULL;
        mapi_pop(U, 1);
    }

    mapi_rotate(U, 2);
    const char *cmd = mapi_get_cstr(U);
    struct pipe_data *in_ptr = create_pipe(U, false);
    struct pipe_data *out_ptr = create_pipe(U, true);
    struct pipe_data *err_ptr = create_pipe(U, true);

    struct pipe_data in = *in_ptr;
    struct pipe_data out = *out_ptr;
    struct pipe_data err = *err_ptr;

    process->pid = fork();
    if (process->pid < 0) {
        mapi_error(U, "unable to create fork");
    } else if (process->pid == 0) { // child
        dup2(in.input, STDIN_FILENO);
        dup2(out.output, STDOUT_FILENO);
        dup2(err.output, STDERR_FILENO);

        close(in.input);
        close(in.output);
        close(out.input);
        close(out.output);
        close(err.input);
        close(err.output);

        if (env) {
            execle("/bin/sh", "/bin/sh", "-c", cmd, NULL, process->env.array);
        } else {
            execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
        }
        _exit(0);
    } else { // parent
        close(in_ptr->input);
        in_ptr->input_closed = true;

        close(out_ptr->output);
        out_ptr->output_closed = true;

        close(err_ptr->output);
        err_ptr->output_closed = true;

        mapi_rotate(U, 5);
        mapi_rotate(U, 5);
        mapi_rotate(U, 5);
        mapi_pop(U, 1);
        mapi_rotate(U, 4);
    }
}

MORPHINE_API void mlapi_process_kill(morphine_coroutine_t U, bool force) {
    process_type(U);

    struct process *process = mapi_userdata_pointer(U, MLIB_PROCESS_USERDATA_TYPE);

    if (!mlapi_process_isalive(U)) {
        return;
    }

    int result = kill(process->pid, force ? SIGKILL : SIGTERM);
    if (result < 0) {
        mapi_error(U, "unable to kill process");
    }

    mlapi_process_wait(U);
}

MORPHINE_API int mlapi_process_wait(morphine_coroutine_t U) {
    process_type(U);

    struct process *process = mapi_userdata_pointer(U, MLIB_PROCESS_USERDATA_TYPE);
    if (!process->status.has) {
        pid_t result = waitpid(process->pid, &process->status.value, 0);
        if (result < 0) {
            mapi_error(U, "unable to wait process");
        }

        process->status.has = true;
    }

    return WEXITSTATUS(process->status.value);
}

MORPHINE_API bool mlapi_process_isalive(morphine_coroutine_t U) {
    process_type(U);

    struct process *process = mapi_userdata_pointer(U, MLIB_PROCESS_USERDATA_TYPE);

    if (process->status.has) {
        return false;
    }

    pid_t result = waitpid(process->pid, &process->status.value, WNOHANG);
    if (result < 0) {
        mapi_error(U, "unable to check process alive");
    }

    if (result != 0) {
        process->status.has = true;
        return false;
    }

    return true;
}
