//
// Created by whyiskra on 30.01.24.
//

#include "instance.h"
#include "jniutils.h"
#include "lib.h"
#include "env.h"
#include "jnistream.h"
#include <morphinec.h>
#include <morphinel.h>
#include <setjmp.h>
#include <malloc.h>

static jint get_jint_field(JNIEnv *jnienv, jobject this, const char *name) {
    jclass class = J(jnienv, GetObjectClass, this);
    jfieldID id = J(jnienv, GetFieldID, class, name, "I");
    jint result = J(jnienv, GetIntField, this, id);
    J(jnienv, DeleteLocalRef, class);

    return result;
}

static size_t jint2size(jint value, bool *error) {
    return mm_overflow_opc_cast(size_t, value, *error = true);
}

static ml_size jint2mlsize(jint value, bool *error) {
    return mm_overflow_opc_cast(ml_size, value, *error = true);
}

static void jstring2mlstring(morphine_coroutine_t U, JNIEnv *jnienv, jstring text) {
    ml_size size =
        mm_overflow_opc_cast(ml_size, J(jnienv, GetStringUTFLength, text), mapi_error(U, "too large string"));
    const char *chars = J(jnienv, GetStringUTFChars, text, NULL);

    mapi_push_stringn(U, chars, size);

    J(jnienv, ReleaseStringUTFChars, text, chars);
}

static morphine_settings_t morphine_settings(JNIEnv *jnienv, jobject this, const char *name, bool *error) {
    jclass class = J(jnienv, GetObjectClass, this);
    jfieldID settings_id = J(jnienv, GetFieldID, class, name, "Lru/why/morphine/jni/Morphine$Settings;");

    jobject settings = J(jnienv, GetObjectField, this, settings_id);
    morphine_settings_t result = {
        .gc.limit = jint2size(get_jint_field(jnienv, settings, "gcLimit"), error),
        .gc.threshold = jint2size(get_jint_field(jnienv, settings, "gcThreshold"), error),
        .gc.grow = jint2size(get_jint_field(jnienv, settings, "gcGrow"), error),
        .gc.deal = jint2size(get_jint_field(jnienv, settings, "gcDeal"), error),
        .gc.pause = jint2size(get_jint_field(jnienv, settings, "gcPause"), error),
        .coroutines.stack.limit = jint2mlsize(get_jint_field(jnienv, settings, "coroutinesStackLimit"), error)
    };

    J(jnienv, DeleteLocalRef, class);
    J(jnienv, DeleteLocalRef, settings);

    return result;
}

JNIEXPORT void JNICALL Java_ru_why_morphine_jni_Morphine_compiler(
    JNIEnv *jnienv,
    jobject this,
    jobject output,
    jstring text
) {
    struct env env = env_init(jnienv, this);
    jmp_env(&env, return, return)

    bool settings_error = false;
    morphine_settings_t settings = morphine_settings(jnienv, this, "compilerSettings", &settings_error);
    if (settings_error) {
        J(env.jnienv, ThrowNew, env.exception.other, "wrong settings parameter");
        return;
    }

    morphine_platform_t platform = env_platform();

    morphine_instance_t I = mapi_open(platform, settings, &env);
    jniutils_check_exception(I, jnienv);

    morphine_coroutine_t U = mapi_coroutine(I);

    push_jnistream(U, jnienv, NULL, output, false);
    jstring2mlstring(U, jnienv, text);
    mcapi_compile(U, "jnimain", false);
    mapi_pack(U);

    mapi_close(I);
}

JNIEXPORT void JNICALL Java_ru_why_morphine_jni_Morphine_interpreter(
    JNIEnv *jnienv,
    jobject this,
    jobject input
) {
    struct env env = env_init(jnienv, this);
    jmp_env(&env, return, return)

    bool settings_error = false;
    morphine_settings_t settings = morphine_settings(jnienv, this, "interpreterSettings", &settings_error);
    if (settings_error) {
        J(env.jnienv, ThrowNew, env.exception.other, "wrong settings parameter");
        return;
    }

    morphine_platform_t platform = env_platform();

    morphine_instance_t I = mapi_open(platform, settings, &env);
    jniutils_check_exception(I, jnienv);

    mlapi_import_all(I);
    mapi_library_load(I, mclib_compiler());
    mapi_library_load(I, mjlib_jni());

    morphine_coroutine_t U = mapi_coroutine(I);

    push_jnistream(U, jnienv, input, NULL, false);
    mapi_unpack(U);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);

    mapi_call(U, 0);
    mapi_interpreter(I);
    mapi_close(I);
}
