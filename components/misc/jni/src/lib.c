//
// Created by why-iskra on 12.11.2024.
//

#include "lib.h"
#include "env.h"
#include "jnisio.h"
#include "jniutils.h"

#define SHAREDKEY ("morphine-misc-jnilib")

static void lib_channel(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            maux_sharedstorage_get(U, SHAREDKEY, "jnisio");
            maux_nb_return();
    maux_nb_end
}

static void lib_interrupted(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);

            struct env *env = mapi_instance_data(mapi_instance(U));
            jclass thread_class = J(env->jnienv, FindClass, "java/lang/Thread");
            jmethodID interrupted = J(env->jnienv, GetStaticMethodID, thread_class, "interrupted", "()Z");
            jboolean is_interrupted = J(env->jnienv, CallStaticBooleanMethod, thread_class, interrupted);
            J(env->jnienv, DeleteLocalRef, thread_class);

            mapi_push_boolean(U, is_interrupted);
            maux_nb_return();
    maux_nb_end
}

static void lib_exit(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            env_exit(mapi_instance(U));
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("channel", lib_channel),
    MAUX_CONSTRUCT_FUNCTION("interrupted", lib_interrupted),
    MAUX_CONSTRUCT_FUNCTION("exit", lib_exit),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    struct env *env = mapi_instance_data(mapi_instance(U));

    jclass this_class = J(env->jnienv, GetObjectClass, env->this);
    jfieldID send_id = J(env->jnienv, GetFieldID, this_class, "sendStream", "Ljava/io/OutputStream;");
    jfieldID receive_id = J(env->jnienv, GetFieldID, this_class, "receiveStream", "Ljava/io/InputStream;");

    jobject send = J(env->jnienv, GetObjectField, env->this, send_id);
    jobject receive = J(env->jnienv, GetObjectField, env->this, receive_id);

    J(env->jnienv, DeleteLocalRef, this_class);

    push_jnisio(U, env->jnienv, receive, send);
    maux_sharedstorage_set(U, SHAREDKEY, "jnisio");

    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mjlib_jni(void) {
    return (morphine_library_t) {
        .name = "jni",
        .sharedkey = SHAREDKEY,
        .init = library_init
    };
}
