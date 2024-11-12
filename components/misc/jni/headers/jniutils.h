//
// Created by why-iskra on 12.11.2024.
//

#pragma once

#include <jni.h>
#include <morphine.h>

#define J(e, n, ...) ((*(e))->n((e), __VA_ARGS__))

jint jniutils_get_int(JNIEnv *, jobject, const char *);
jobject jniutils_get_obj(JNIEnv *, jobject, const char *, const char *);

size_t jniutils_jint2size(jint);

jsize jniutils_mlsize2jsize(morphine_coroutine_t, ml_size, bool *);
ml_size jniutils_jsize2mlsize(morphine_coroutine_t, jsize, bool *);

void jniutils_jstring2mlstring(morphine_coroutine_t, JNIEnv *, jstring, bool *);
jstring jniutils_mlstring2jstring(morphine_coroutine_t, JNIEnv *);
