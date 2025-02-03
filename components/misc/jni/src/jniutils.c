//
// Created by why-iskra on 12.11.2024.
//

#include "jniutils.h"

bool jniutils_ignore_exception(JNIEnv *jnienv) {
    if ((*jnienv)->ExceptionCheck(jnienv) == JNI_TRUE) {
        (*jnienv)->ExceptionDescribe(jnienv);
        (*jnienv)->ExceptionClear(jnienv);
        return true;
    }

    return false;
}

void jniutils_check_exception(morphine_instance_t I, JNIEnv *jnienv) {
    if (jniutils_ignore_exception(jnienv)) {
        mapi_ierror(I, "jni exception caused");
    }
}
