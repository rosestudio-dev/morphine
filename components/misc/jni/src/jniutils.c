//
// Created by why-iskra on 12.11.2024.
//

#include "jniutils.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

jint jniutils_get_int(JNIEnv *jnienv, jobject this, const char *name) {
    jclass class = J(jnienv, GetObjectClass, this);
    jfieldID id = J(jnienv, GetFieldID, class, name, "I");
    jint result = J(jnienv, GetIntField, this, id);
    J(jnienv, DeleteLocalRef, class);
    return result;
}

jobject jniutils_get_obj(JNIEnv *jnienv, jobject this, const char *name, const char *sig) {
    jclass class = J(jnienv, GetObjectClass, this);
    jfieldID id = J(jnienv, GetFieldID, class, name, sig);
    jobject result = J(jnienv, GetObjectField, this, id);
    J(jnienv, DeleteLocalRef, class);
    return result;
}

size_t jniutils_jint2size(jint value) {
    return value < 0 ? 0 : (size_t) value;
}

jsize jniutils_mlsize2jsize(morphine_coroutine_t U, ml_size size, bool *error) {
    if (size > INT32_MAX) {
        if (error == NULL) {
            mapi_error(U, "cannot convert size as jni");
        } else {
            *error = true;
        }
    }

    if (error != NULL) {
        *error = false;
    }

    return (jsize) size;
}

ml_size jniutils_jsize2mlsize(morphine_coroutine_t U, jsize size, bool *error) {
    if (size < 0) {
        if (error == NULL) {
            mapi_error(U, "cannot convert jni size");
        } else {
            *error = true;
        }
    }

    if (error != NULL) {
        *error = false;
    }

    return (ml_size) size;
}

void jniutils_jstring2mlstring(morphine_coroutine_t U, JNIEnv *jnienv, jstring text, bool *error) {
    ml_size size = jniutils_jsize2mlsize(U, J(jnienv, GetStringUTFLength, text), error);
    const char *chars = J(jnienv, GetStringUTFChars, text, NULL);

    mapi_push_stringn(U, chars, size);

    J(jnienv, ReleaseStringUTFChars, text, chars);
}

jstring jniutils_mlstring2jstring(morphine_coroutine_t U, JNIEnv *jnienv) {
    const char *chars = mapi_get_cstr(U);
    return J(jnienv, NewStringUTF, chars);
}
