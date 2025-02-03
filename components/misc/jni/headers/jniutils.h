//
// Created by why-iskra on 12.11.2024.
//

#pragma once

#include <jni.h>
#include <morphine.h>

#define J(e, n, ...) ((*(e))->n((e), __VA_ARGS__))

bool jniutils_ignore_exception(JNIEnv *);
void jniutils_check_exception(morphine_instance_t, JNIEnv *);
