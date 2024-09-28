//
// Created by why-iskra on 28.09.2024.
//

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include "morphinel/system/popen.h"

struct pipe_data {
    const char *cmd;
    bool closed;

    pid_t pid;
    int input;
    int output;
};

static void *pipe_open(morphine_sio_accessor_t A, void *data) {
    struct pipe_data *D = data;

    int fdin[2], fdout[2];

    if (pipe(fdin) != 0) {
        mapi_sio_accessor_error(A, "failed to open input pipe");
    }

    if (pipe(fdout) != 0) {
        mapi_sio_accessor_error(A, "failed to open output pipe");
    }

    switch (D->pid = fork()) {
        case -1:
            mapi_sio_accessor_error(A, "failed to fork process");
        case 0:
            close(fdin[0]);
            close(fdout[0]);

            dup2(fdin[1], 0);
            dup2(fdout[1], 1);

            close(fdin[1]);
            close(fdout[1]);

            execl("/bin/sh", "sh", "-c", D->cmd, NULL);
            _exit(127);
        default:
            break;
    }

    close(fdin[1]);
    close(fdout[1]);

    D->input = fdin[0];
    D->output = fdout[0];

    D->closed = false;

    return D;
}

static void pipe_close(morphine_sio_accessor_t A, void *data) {
    (void) A;

    struct pipe_data *D = data;

    if (!D->closed) {
        D->closed = true;

        waitpid(D->pid, NULL, 0);

        close(D->input);
        close(D->output);
    }
}

static size_t pipe_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    struct pipe_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "pipe closed");
    }

    ssize_t result = write(D->input, buffer, size);

    if (result < 0) {
        mapi_sio_accessor_error(A, "pipe error");
    }

    return (size_t) result;
}

static size_t pipe_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    struct pipe_data *D = data;

    if (D->closed) {
        mapi_sio_accessor_error(A, "pipe closed");
    }

    ssize_t result = read(D->output, buffer, size);

    if (result < 0) {
        mapi_sio_accessor_error(A, "pipe error");
    }

    return (size_t) result;
}

MORPHINE_API void mlapi_system_popen(morphine_coroutine_t U) {
    const char *cmd = mapi_get_cstr(U);
    struct pipe_data *D = mapi_push_userdata_uni(U, sizeof(struct pipe_data));

    (*D) = (struct pipe_data) {
        .cmd = cmd,
        .input = 0,
        .output = 1,
        .closed = true
    };

    mapi_push_vector(U, 2);

    mapi_rotate(U, 2);
    mapi_vector_set(U, 0);

    mapi_rotate(U, 2);
    mapi_vector_set(U, 1);

    // create sio

    morphine_sio_interface_t interface = {
        .open = pipe_open,
        .close = pipe_close,
        .read = pipe_read,
        .write = pipe_write,
        .flush = NULL,
        .eos = NULL,
        .tell = NULL,
        .seek = NULL
    };

    mapi_push_sio(U, interface);

    mapi_rotate(U, 2);
    mapi_sio_hold(U);

    mapi_sio_open(U, D);
}
