//
// Created by why-iskra on 12.11.2024.
//

#pragma once

#include <jni.h>
#include <morphine.h>

void push_jnisio(morphine_coroutine_t, JNIEnv *, jobject input, jobject output, bool remove_local_ref);
