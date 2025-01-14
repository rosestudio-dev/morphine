//
// Created by why-iskra on 12.11.2024.
//

#include "jnistream.h"
#include "jniutils.h"

struct jnistream_data {
    JNIEnv *jnienv;
    jobject input;
    jobject output;
    jmethodID read_id;
    jmethodID write_id;
    jmethodID flush_id;
    jmethodID available_id;

    bool remove_local_ref;
};

static void buf_open(morphine_instance_t I, void *data, void *args) {
    (void) I;
    struct jnistream_data *jnistream_data = data;
    struct jnistream_data *jnistream_args = args;

    (*jnistream_data) = (*jnistream_args);
}

static void buf_close(morphine_instance_t I, void *data) {
    (void) I;
    struct jnistream_data *jnistream_data = data;

    if (jnistream_data->remove_local_ref && jnistream_data->input != NULL) {
        J(jnistream_data->jnienv, DeleteLocalRef, jnistream_data->input);
    }

    if (jnistream_data->remove_local_ref && jnistream_data->output != NULL) {
        J(jnistream_data->jnienv, DeleteLocalRef, jnistream_data->output);
    }
}

static size_t buf_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    (void) I;
    struct jnistream_data *D = data;

    size_t success = 0;
    for (size_t i = 0; i < size; i++) {
        jint result = J(D->jnienv, CallIntMethod, D->input, D->read_id);
        if (jniutils_ignore_exception(D->jnienv) || result < 0) {
            break;
        }

        buffer[i] = (uint8_t) result;
        success++;
    }

    return success;
}

static bool buf_eos(morphine_instance_t I, void *data) {
    (void) I;
    struct jnistream_data *D = data;

    jint result = J(D->jnienv, CallIntMethod, D->input, D->available_id);
    jniutils_check_exception(I, D->jnienv);

    return result == 0;
}

static size_t buf_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    (void) I;
    struct jnistream_data *D = data;

    for (size_t i = 0; i < size; i++) {
        J(D->jnienv, CallVoidMethod, D->output, D->write_id, (jint) buffer[i]);

        if (jniutils_ignore_exception(D->jnienv)) {
            return i;
        }
    }

    return size;
}

static void buf_flush(morphine_instance_t I, void *data) {
    (void) I;
    struct jnistream_data *D = data;

    J(D->jnienv, CallVoidMethod, D->output, D->flush_id);
    jniutils_check_exception(I, D->jnienv);
}

void push_jnistream(morphine_coroutine_t U, JNIEnv *jnienv, jobject input, jobject output, bool remove_local_ref) {
    jmethodID read_id = 0;
    jmethodID write_id = 0;
    jmethodID flush_id = 0;
    jmethodID available_id = 0;

    if (input != NULL) {
        jclass input_class = J(jnienv, GetObjectClass, input);
        jniutils_check_exception(mapi_instance(U), jnienv);

        read_id = J(jnienv, GetMethodID, input_class, "read", "()I");
        available_id = J(jnienv, GetMethodID, input_class, "available", "()I");
        J(jnienv, DeleteLocalRef, input_class);
    }

    if (output != NULL) {
        jclass output_class = J(jnienv, GetObjectClass, output);
        jniutils_check_exception(mapi_instance(U), jnienv);

        write_id = J(jnienv, GetMethodID, output_class, "write", "(I)V");
        flush_id = J(jnienv, GetMethodID, output_class, "flush", "()V");
        J(jnienv, DeleteLocalRef, output_class);
    }

    morphine_stream_interface_t interface = {
        .data_size = sizeof(struct jnistream_data),
        .open = buf_open,
        .close = buf_close,
        .read = input != NULL ? buf_read : NULL,
        .write = output != NULL ? buf_write : NULL,
        .flush = output != NULL ? buf_flush : NULL,
        .eos = input != NULL ? buf_eos : NULL,
        .tell = NULL,
        .seek = NULL,
    };

    struct jnistream_data args = {
        .jnienv = jnienv,
        .input = input,
        .output = output,
        .read_id = read_id,
        .write_id = write_id,
        .flush_id = flush_id,
        .available_id = available_id,
        .remove_local_ref = remove_local_ref,
    };

    mapi_push_stream(U, interface, false, &args);
}
