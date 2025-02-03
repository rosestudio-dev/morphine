//
// Created by why-iskra on 24.09.2024.
//

#include <string.h>
#include <malloc.h>
#include "env.h"
#include "jniutils.h"

static size_t platform_io_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    (void) data;
    struct env *env = mapi_instance_data(I);

    for (size_t i = 0; i < size; i++) {
        J(env->jnienv, CallVoidMethod, env->output.object, env->output.write_id, (jint) buffer[i]);

        if (jniutils_ignore_exception(env->jnienv)) {
            return i;
        }
    }

    return size;
}

static size_t platform_io_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    (void) data;
    struct env *env = mapi_instance_data(I);

    size_t success = 0;
    for (size_t i = 0; i < size; i++) {
        jint result = J(env->jnienv, CallIntMethod, env->input.object, env->input.read_id);
        if (jniutils_ignore_exception(env->jnienv) || result < 0) {
            break;
        }

        buffer[i] = (uint8_t) result;
        success++;
    }

    return success;
}

static void platform_io_flush(morphine_instance_t I, void *data) {
    (void) data;
    struct env *env = mapi_instance_data(I);
    J(env->jnienv, CallVoidMethod, env->output.object, env->output.flush_id);
    jniutils_check_exception(I, env->jnienv);
}

static size_t platform_io_error_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    (void) data;
    struct env *env = mapi_instance_data(I);

    for (size_t i = 0; i < size; i++) {
        J(env->jnienv, CallVoidMethod, env->error.object, env->error.write_id, (jint) buffer[i]);

        if (jniutils_ignore_exception(env->jnienv)) {
            return i;
        }
    }

    return size;
}

static void platform_io_error_flush(morphine_instance_t I, void *data) {
    (void) data;
    struct env *env = mapi_instance_data(I);
    J(env->jnienv, CallVoidMethod, env->error.object, env->error.flush_id);
    jniutils_check_exception(I, env->jnienv);
}

static void *platform_alloc(void *data, size_t size) {
    (void) data;
    return malloc(size);
}

static void *platform_realloc(void *data, void *pointer, size_t size) {
    (void) data;
    return realloc(pointer, size);
}

static void platform_free(void *data, void *pointer) {
    (void) data;
    free(pointer);
}

morphine_noret static void platform_signal(morphine_instance_t I, void *data, bool is_panic) {
    struct env *env = data;
    env->exit.I = NULL;

    J(env->jnienv, ThrowNew, is_panic ? env->exception.panic : env->exception.error, mapi_signal_message(I));

    if (I != NULL && !mapi_is_nested_signal(I) && !is_panic) {
        mapi_close(I);
    }

    longjmp(env->exit.jmp, 1);
}

struct env env_init(JNIEnv *jnienv, jobject this) {
    struct env env = {
        .jnienv = jnienv
    };

    jclass this_class = J(jnienv, GetObjectClass, this);

    {
        jfieldID bridge_id = J(jnienv, GetFieldID, this_class, "bridge", "Lru/why/morphine/jni/Morphine$Bridge;");
        jobject bridge = J(jnienv, GetObjectField, this, bridge_id);

        jclass bridge_class = J(jnienv, GetObjectClass, bridge);
        jfieldID input_id = J(jnienv, GetFieldID, bridge_class, "inputStream", "Ljava/io/InputStream;");
        jfieldID output_id = J(jnienv, GetFieldID, bridge_class, "outputStream", "Ljava/io/OutputStream;");
        jfieldID error_id = J(jnienv, GetFieldID, bridge_class, "errorStream", "Ljava/io/OutputStream;");

        env.input.object = J(jnienv, GetObjectField, bridge, input_id);
        jclass input_class = J(jnienv, GetObjectClass, env.input.object);
        env.input.read_id = J(jnienv, GetMethodID, input_class, "read", "()I");

        env.output.object = J(jnienv, GetObjectField, bridge, output_id);
        jclass output_class = J(jnienv, GetObjectClass, env.output.object);
        env.output.write_id = J(jnienv, GetMethodID, output_class, "write", "(I)V");
        env.output.flush_id = J(jnienv, GetMethodID, output_class, "flush", "()V");

        env.error.object = J(jnienv, GetObjectField, bridge, error_id);
        jclass error_class = J(jnienv, GetObjectClass, env.error.object);
        env.error.write_id = J(jnienv, GetMethodID, error_class, "write", "(I)V");
        env.error.flush_id = J(jnienv, GetMethodID, error_class, "flush", "()V");

        J(jnienv, DeleteLocalRef, bridge);
        J(jnienv, DeleteLocalRef, bridge_class);
        J(jnienv, DeleteLocalRef, input_class);
        J(jnienv, DeleteLocalRef, output_class);
        J(jnienv, DeleteLocalRef, error_class);
    }

    {
        env.thread.clazz = J(jnienv, FindClass, "java/lang/Thread");
        env.thread.interrupted_id = J(jnienv, GetStaticMethodID, env.thread.clazz, "interrupted", "()Z");
    }

    env.exception.error = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineException$Error");
    env.exception.panic = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineException$Panic");

    env.value.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue");

    {
        env.value.nil.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue$Nil");
        jfieldID value_nil_instance_id =
            J(jnienv, GetStaticFieldID, env.value.nil.clazz, "INSTANCE", "Lru/why/morphine/jni/MorphineValue$Nil;");
        env.value.nil.object = J(jnienv, GetStaticObjectField, env.value.nil.clazz, value_nil_instance_id);

        env.value.integer.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue$Integer");
        env.value.integer.constructor_id = J(jnienv, GetMethodID, env.value.integer.clazz, "<init>", "(J)V");
        env.value.integer.value_id = J(jnienv, GetFieldID, env.value.integer.clazz, "value", "J");

        env.value.decimal.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue$Decimal");
        env.value.decimal.constructor_id = J(jnienv, GetMethodID, env.value.decimal.clazz, "<init>", "(D)V");
        env.value.decimal.value_id = J(jnienv, GetFieldID, env.value.decimal.clazz, "value", "D");

        env.value.boolean.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue$Boolean");
        env.value.boolean.constructor_id = J(jnienv, GetMethodID, env.value.boolean.clazz, "<init>", "(Z)V");
        env.value.boolean.value_id = J(jnienv, GetFieldID, env.value.boolean.clazz, "value", "Z");

        env.value.string.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue$String");
        env.value.string.constructor_id =
            J(jnienv, GetMethodID, env.value.string.clazz, "<init>", "(Ljava/lang/String;)V");
        env.value.string.value_id = J(jnienv, GetFieldID, env.value.string.clazz, "value", "Ljava/lang/String;");

        env.value.table.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue$Table");
        env.value.table.constructor_id =
            J(jnienv, GetMethodID, env.value.table.clazz, "<init>", "(Ljava/util/Map;)V");
        env.value.table.value_id = J(jnienv, GetFieldID, env.value.table.clazz, "value", "Ljava/util/Map;");

        env.value.vector.clazz = J(jnienv, FindClass, "ru/why/morphine/jni/MorphineValue$Vector");
        env.value.vector.constructor_id =
            J(jnienv, GetMethodID, env.value.vector.clazz, "<init>", "(Ljava/util/List;)V");
        env.value.vector.value_id = J(jnienv, GetFieldID, env.value.vector.clazz, "value", "Ljava/util/List;");
    }

    {
        jfieldID callable_id = J(jnienv, GetFieldID, this_class, "callable", "Lru/why/morphine/jni/Morphine$Callable;");
        env.callable.object = J(jnienv, GetObjectField, this, callable_id);

        jclass callable_class = J(jnienv, GetObjectClass, env.callable.object);
        env.callable.call_id = J(
            jnienv, GetMethodID, callable_class, "call",
            "(Ljava/lang/String;[Lru/why/morphine/jni/MorphineValue;)Lru/why/morphine/jni/MorphineValue;"
        );

        J(jnienv, DeleteLocalRef, callable_class);
    }

    {
        env.array_list.clazz = J(jnienv, FindClass, "java/util/ArrayList");
        env.array_list.constructor_id = J(jnienv, GetMethodID, env.array_list.clazz, "<init>", "(I)V");
        env.array_list.add_id = J(jnienv, GetMethodID, env.array_list.clazz, "add", "(Ljava/lang/Object;)Z");
        env.array_list.get_id = J(jnienv, GetMethodID, env.array_list.clazz, "get", "(I)Ljava/lang/Object;");
        env.array_list.size_id = J(jnienv, GetMethodID, env.array_list.clazz, "size", "()I");
    }

    {
        env.hash_map.clazz = J(jnienv, FindClass, "java/util/HashMap");
        env.hash_map.constructor_id = J(jnienv, GetMethodID, env.hash_map.clazz, "<init>", "(I)V");
        env.hash_map.put_id = J(
            jnienv, GetMethodID, env.hash_map.clazz, "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"
        );
        env.hash_map.entry_set_id = J(jnienv, GetMethodID, env.hash_map.clazz, "entrySet", "()Ljava/util/Set;");
    }

    {
        jclass set_class = J(jnienv, FindClass, "java/util/Set");
        env.set.iterator_id = J(jnienv, GetMethodID, set_class, "iterator", "()Ljava/util/Iterator;");
        J(jnienv, DeleteLocalRef, set_class);
    }

    {
        jclass iterator_class = J(jnienv, FindClass, "java/util/Iterator");
        env.iterator.has_next_id = J(jnienv, GetMethodID, iterator_class, "hasNext", "()Z");
        env.iterator.next_id = J(jnienv, GetMethodID, iterator_class, "next", "()Ljava/lang/Object;");
        J(jnienv, DeleteLocalRef, iterator_class);
    }

    {
        jclass entry_class = J(jnienv, FindClass, "java/util/Map$Entry");
        env.map_entry.get_key_id = J(jnienv, GetMethodID, entry_class, "getKey", "()Ljava/lang/Object;");
        env.map_entry.get_value_id = J(jnienv, GetMethodID, entry_class, "getValue", "()Ljava/lang/Object;");
        J(jnienv, DeleteLocalRef, entry_class);
    }

    J(jnienv, DeleteLocalRef, this_class);

    return env;
}

morphine_platform_t env_platform(void) {
    return (morphine_platform_t) {
        .signal = platform_signal,
        .memory.alloc = platform_alloc,
        .memory.realloc = platform_realloc,
        .memory.free = platform_free,
        .stream.io = maux_stream_interface_srwf(platform_io_read, platform_io_write, platform_io_flush),
        .stream.err = maux_stream_interface_swf(platform_io_error_write, platform_io_error_flush),
    };
}

morphine_noret void env_exit(morphine_instance_t I) {
    struct env *env = mapi_instance_data(I);

    env->exit.I = I;
    longjmp(env->exit.jmp, 1);
}
