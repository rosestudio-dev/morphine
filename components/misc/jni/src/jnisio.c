//
// Created by why-iskra on 12.11.2024.
//

#include "jnisio.h"
#include "jniutils.h"

#define USERDATA_TYPE ("jnistream-data")

struct jnisio_data {
    JNIEnv *jnienv;
    jobject input;
    jobject output;
    jmethodID read_id;
    jmethodID write_id;
    jmethodID flush_id;
};

static void jnisio_data_init(morphine_instance_t I, void *data) {
    (void) I;
    struct jnisio_data *D = data;
    (*D) = (struct jnisio_data) {
        .jnienv = NULL,
        .input = NULL,
        .output = NULL
    };
}

static void jnisio_data_free(morphine_instance_t I, void *data) {
    (void) I;
    struct jnisio_data *D = data;
    if (D->jnienv == NULL) {
        return;
    }

    if (D->input != NULL) {
        J(D->jnienv, DeleteLocalRef, D->input);
    }

    if (D->output != NULL) {
        J(D->jnienv, DeleteLocalRef, D->output);
    }
}

static struct jnisio_data *push_jnisio_data(
    morphine_coroutine_t U,
    JNIEnv *jnienv,
    jobject input,
    jobject output
) {
    mapi_type_declare(
        mapi_instance(U),
        USERDATA_TYPE,
        sizeof(struct jnisio_data),
        jnisio_data_init,
        jnisio_data_free,
        NULL,
        NULL,
        false
    );

    struct jnisio_data *D = mapi_push_userdata(U, USERDATA_TYPE);

    jmethodID read_id = 0;
    jmethodID write_id = 0;
    jmethodID flush_id = 0;

    if (input != NULL) {
        jclass input_class = J(jnienv, GetObjectClass, input);
        read_id = J(jnienv, GetMethodID, input_class, "read", "()I");
        J(jnienv, DeleteLocalRef, input_class);
    }

    if (output != NULL) {
        jclass output_class = J(jnienv, GetObjectClass, output);
        write_id = J(jnienv, GetMethodID, output_class, "write", "(I)V");
        flush_id = J(jnienv, GetMethodID, output_class, "flush", "()V");
        J(jnienv, DeleteLocalRef, output_class);
    }

    (*D) = (struct jnisio_data) {
        .jnienv = jnienv,
        .input = input,
        .output = output,
        .read_id = read_id,
        .write_id = write_id,
        .flush_id = flush_id
    };

    return D;
}

static size_t buf_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    (void) A;
    struct jnisio_data *D = data;

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
    struct jnisio_data *D = data;

    for (size_t i = 0; i < size; i++) {
        J(D->jnienv, CallVoidMethod, D->output, D->write_id, (jint) buffer[i]);
    }

    return size;
}

static void buf_flush(morphine_sio_accessor_t A, void *data) {
    (void) A;
    struct jnisio_data *D = data;

    J(D->jnienv, CallVoidMethod, D->output, D->flush_id);
}

void push_jnisio(morphine_coroutine_t U, JNIEnv *jnienv, jobject input, jobject output) {
    morphine_sio_interface_t interface = {
        .open = NULL,
        .close = NULL,
        .read = input != NULL ? buf_read : NULL,
        .write = output != NULL ? buf_write : NULL,
        .flush = output != NULL ? buf_flush : NULL,
        .eos = NULL,
        .tell = NULL,
        .seek = NULL
    };

    struct jnisio_data *D = push_jnisio_data(U, jnienv, input, output);

    mapi_push_sio(U, interface);
    mapi_rotate(U, 2);
    mapi_sio_hold(U);

    mapi_sio_open(U, D);
}
