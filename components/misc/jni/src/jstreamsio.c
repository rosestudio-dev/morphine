//
// Created by why-iskra on 12.11.2024.
//

#include "jstreamsio.h"
#include "jniutils.h"

struct buf_data {
    JNIEnv *jnienv;
    jobject input;
    jobject output;
    jmethodID read_id;
    jmethodID write_id;
};

static size_t buf_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    (void) A;
    struct buf_data *D = data;

    size_t success = 0;
    for (size_t i = 0; i < size; i++) {
        jint result = J(D->jnienv, CallIntMethod, D->input, D->read_id);

        if (result < 0) {
            break;
        }

        buffer[i] = (uint8_t) result;
        success++;
    }

    return success;
}

static size_t buf_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    (void) A;
    struct buf_data *D = data;

    for (size_t i = 0; i < size; i++) {
        J(D->jnienv, CallVoidMethod, D->output, D->write_id, (jint) buffer[i]);
    }

    return size;
}

void jstreamsio_push(morphine_coroutine_t U, JNIEnv *jnienv, jobject input, jobject output) {
    jmethodID read_id = 0;
    jmethodID write_id = 0;

    if (input != NULL) {
        jclass input_class = J(jnienv, GetObjectClass, input);
        read_id = J(jnienv, GetMethodID, input_class, "read", "()I");
        J(jnienv, DeleteLocalRef, input_class);
    }

    if (output != NULL) {
        jclass output_class = J(jnienv, GetObjectClass, output);
        write_id = J(jnienv, GetMethodID, output_class, "write", "(I)V");
        J(jnienv, DeleteLocalRef, output_class);
    }

    struct buf_data *D = mapi_push_userdata_uni(U, sizeof(struct buf_data));

    (*D) = (struct buf_data) {
        .jnienv = jnienv,
        .input = input,
        .output = output,
        .read_id = read_id,
        .write_id = write_id
    };

    morphine_sio_interface_t interface = {
        .open = NULL,
        .close = NULL,
        .read = input != NULL ? buf_read : NULL,
        .write = output != NULL ? buf_write : NULL,
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
