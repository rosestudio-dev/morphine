//
// Created by whyiskra on 30.01.24.
//

#include "instance.h"
#include "jniutils.h"
#include "jnisio.h"
#include "lib.h"
#include "env.h"
#include <morphinec.h>
#include <morphinel.h>
#include <setjmp.h>
#include <malloc.h>

static size_t io_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    (void) A;
    struct env *env = data;

    for (size_t i = 0; i < size; i++) {
        J(env->jnienv, CallVoidMethod, env->output.object, env->output.write_id, (jint) buffer[i]);
    }

    return size;
}

static size_t io_read(morphine_sio_accessor_t A, void *data, uint8_t *buffer, size_t size) {
    (void) A;
    struct env *env = data;

    size_t success = 0;
    for (size_t i = 0; i < size; i++) {
        jint result = J(env->jnienv, CallIntMethod, env->input.object, env->input.read_id);

        if (result < 0) {
            break;
        }

        buffer[i] = (uint8_t) result;
        success++;
    }

    return success;
}

static void io_flush(morphine_sio_accessor_t A, void *data) {
    (void) A;
    struct env *env = data;
    J(env->jnienv, CallVoidMethod, env->output.object, env->output.flush_id);
}

static size_t io_error_write(morphine_sio_accessor_t A, void *data, const uint8_t *buffer, size_t size) {
    (void) A;
    struct env *env = data;

    for (size_t i = 0; i < size; i++) {
        J(env->jnienv, CallVoidMethod, env->error.object, env->error.write_id, (jint) buffer[i]);
    }

    return size;
}

static void io_error_flush(morphine_sio_accessor_t A, void *data) {
    (void) A;
    struct env *env = data;
    J(env->jnienv, CallVoidMethod, env->error.object, env->error.flush_id);
}

static void *vmmalloc(void *data, size_t size) {
    (void) data;
    return malloc(size);
}

static void *vmrealloc(void *data, void *pointer, size_t size) {
    (void) data;
    return realloc(pointer, size);
}

static void vmfree(void *data, void *pointer) {
    (void) data;
    free(pointer);
}

static morphine_settings_t morphine_settings(JNIEnv *jnienv, jobject this) {
    jclass class = J(jnienv, GetObjectClass, this);

    jfieldID settings_id =
        J(jnienv, GetFieldID, class, "settings", "Lru/why/morphine/jni/Morphine$Settings;");

    jobject settings = J(jnienv, GetObjectField, this, settings_id);

    morphine_settings_t result = {
        .gc.limit = jniutils_jint2size(jniutils_get_int(jnienv, settings, "gcLimit")),
        .gc.threshold = jniutils_jint2size(jniutils_get_int(jnienv, settings, "gcThreshold")),
        .gc.grow = (uint16_t) jniutils_jint2size(jniutils_get_int(jnienv, settings, "gcGrow")),
        .gc.deal = (uint16_t) jniutils_jint2size(jniutils_get_int(jnienv, settings, "gcDeal")),
        .gc.pause = (uint8_t) jniutils_jint2size(jniutils_get_int(jnienv, settings, "gcPause")),
        .gc.cache.callinfo = jniutils_jint2size(jniutils_get_int(jnienv, settings, "gcCacheCallinfo")),
        .coroutines.stack.limit =
        jniutils_jint2size(jniutils_get_int(jnienv, settings, "coroutinesStackLimit"))
    };

    J(jnienv, DeleteLocalRef, class);
    J(jnienv, DeleteLocalRef, settings);

    return result;
}

static struct env init(JNIEnv *jnienv, jobject this) {
    jclass this_class = J(jnienv, GetObjectClass, this);
    jfieldID bridge_id = J(jnienv, GetFieldID, this_class, "bridge", "Lru/why/morphine/jni/Morphine$Bridge;");

    jobject bridge = J(jnienv, GetObjectField, this, bridge_id);
    jclass bridge_class = J(jnienv, GetObjectClass, bridge);
    jfieldID input_id = J(jnienv, GetFieldID, bridge_class, "inputStream", "Ljava/io/InputStream;");
    jfieldID output_id = J(jnienv, GetFieldID, bridge_class, "outputStream", "Ljava/io/OutputStream;");
    jfieldID error_id = J(jnienv, GetFieldID, bridge_class, "errorStream", "Ljava/io/OutputStream;");

    jobject input = J(jnienv, GetObjectField, bridge, input_id);
    jclass input_class = J(jnienv, GetObjectClass, input);
    jmethodID input_read = J(jnienv, GetMethodID, input_class, "read", "()I");

    jobject output = J(jnienv, GetObjectField, bridge, output_id);
    jclass output_class = J(jnienv, GetObjectClass, output);
    jmethodID output_write = J(jnienv, GetMethodID, output_class, "write", "(I)V");
    jmethodID output_flush = J(jnienv, GetMethodID, output_class, "flush", "()V");

    jobject error = J(jnienv, GetObjectField, bridge, error_id);
    jclass error_class = J(jnienv, GetObjectClass, error);
    jmethodID error_write = J(jnienv, GetMethodID, error_class, "write", "(I)V");
    jmethodID error_flush = J(jnienv, GetMethodID, error_class, "flush", "()V");

    J(jnienv, DeleteLocalRef, bridge);
    J(jnienv, DeleteLocalRef, this_class);
    J(jnienv, DeleteLocalRef, bridge_class);
    J(jnienv, DeleteLocalRef, input_class);
    J(jnienv, DeleteLocalRef, output_class);
    J(jnienv, DeleteLocalRef, error_class);

    return (struct env) {
        .jnienv = jnienv,
        .this = this,
        .input.object = input,
        .input.read_id = input_read,
        .output.object = output,
        .output.write_id = output_write,
        .output.flush_id = output_flush,
        .error.object = error,
        .error.write_id = error_write,
        .error.flush_id = error_flush,
    };
}

JNIEXPORT jboolean JNICALL Java_ru_why_morphine_jni_Morphine_compiler(
    JNIEnv *jnienv,
    jobject this,
    jobject output,
    jstring text
) {
    struct env env = init(jnienv, this);
    jmp_env(&env, return false, return true)

    morphine_settings_t settings = morphine_settings(jnienv, this);
    morphine_platform_t platform = {
        .functions.malloc = vmmalloc,
        .functions.realloc = vmrealloc,
        .functions.free = vmfree,
        .functions.signal = env_signal,
        .sio_io_interface = maux_sio_interface_srwf(io_read, io_write, io_flush),
        .sio_error_interface = maux_sio_interface_swf(io_error_write, io_error_flush),
    };

    morphine_instance_t I = mapi_open(platform, settings, &env);
    morphine_coroutine_t U = mapi_coroutine(I, "jnicmp");

    push_jnisio(U, jnienv, NULL, output);
    jniutils_jstring2mlstring(U, jnienv, text, NULL);
    mcapi_compile(U, "jnimain", false, false);
    mapi_binary_to(U);

    mapi_close(I);

    return false;
}

JNIEXPORT jboolean JNICALL Java_ru_why_morphine_jni_Morphine_interpreter(
    JNIEnv *jnienv,
    jobject this,
    jobject input
) {
    struct env env = init(jnienv, this);
    jmp_env(&env, return false, return true)

    morphine_settings_t settings = morphine_settings(jnienv, this);
    morphine_platform_t platform = {
        .functions.malloc = vmmalloc,
        .functions.realloc = vmrealloc,
        .functions.free = vmfree,
        .functions.signal = env_signal,
        .sio_io_interface = maux_sio_interface_srwf(io_read, io_write, io_flush),
        .sio_error_interface = maux_sio_interface_swf(io_error_write, io_error_flush),
    };

    morphine_instance_t I = mapi_open(platform, settings, &env);
    mlapi_import_all(I);
    mapi_library_load(I, mclib_compiler());
    mapi_library_load(I, mjlib_jni());

    morphine_coroutine_t U = mapi_coroutine(I, "jniexec");

    push_jnisio(U, jnienv, input, NULL);
    mapi_binary_from(U);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);

    mapi_call(U, 0);
    mapi_interpreter(I);
    mapi_close(I);

    return false;
}
