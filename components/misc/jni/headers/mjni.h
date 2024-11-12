//
// Created by why-iskra on 12.11.2024.
//

#pragma once

#include <jni.h>

/*
 * Class:     ru_why_morphine_jni_Morphine
 * Method:    compiler
 * Signature: (Ljava/io/OutputStream;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_ru_why_morphine_jni_Morphine_compiler(JNIEnv *, jobject, jobject, jstring);

/*
 * Class:     ru_why_morphine_jni_Morphine
 * Method:    interpreter
 * Signature: (Ljava/io/InputStream;)Z
 */
JNIEXPORT jboolean JNICALL Java_ru_why_morphine_jni_Morphine_interpreter(JNIEnv *, jobject, jobject);
