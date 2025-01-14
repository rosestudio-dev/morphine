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

static jint jniint(JNIEnv *jnienv, jobject this, const char *name) {
    jclass class = J(jnienv, GetObjectClass, this);
    jfieldID id = J(jnienv, GetFieldID, class, name, "I");
    jint result = J(jnienv, GetIntField, this, id);
    J(jnienv, DeleteLocalRef, class);

    return result;
}

static morphine_settings_t morphine_settings(JNIEnv *jnienv, jobject this) {
    jclass class = J(jnienv, GetObjectClass, this);
    jfieldID settings_id = J(jnienv, GetFieldID, class, "settings", "Lru/why/morphine/jni/Morphine$Settings;");

    jobject settings = J(jnienv, GetObjectField, this, settings_id);
    morphine_settings_t result = {
        .gc.limit = jniutils_jint2size(jniint(jnienv, settings, "gcLimit")),
        .gc.threshold = jniutils_jint2size(jniint(jnienv, settings, "gcThreshold")),
        .gc.grow = (uint16_t) jniutils_jint2size(jniint(jnienv, settings, "gcGrow")),
        .gc.deal = (uint16_t) jniutils_jint2size(jniint(jnienv, settings, "gcDeal")),
        .gc.pause = (uint8_t) jniutils_jint2size(jniint(jnienv, settings, "gcPause")),
        .gc.cache.callinfo = jniutils_jint2size(jniint(jnienv, settings, "gcCacheCallinfo")),
        .coroutines.stack.limit = jniutils_jint2size(jniint(jnienv, settings, "coroutinesStackLimit"))
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

    morphine_settings_t settings = morphine_settings(jnienv, this);
    morphine_platform_t platform = env_platform();

    morphine_instance_t I = mapi_open(platform, settings, &env);
    jniutils_check_exception(I, jnienv);

    morphine_coroutine_t U = mapi_coroutine(I);

    push_jnistream(U, jnienv, NULL, output, false);
    jniutils_jstring2mlstring(U, jnienv, text, NULL);
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

    morphine_settings_t settings = morphine_settings(jnienv, this);
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
    mapi_attach(U);

    mapi_interpreter(I);
    mapi_close(I);
}
