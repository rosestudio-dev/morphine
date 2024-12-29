//
// Created by why-iskra on 12.11.2024.
//

#include "lib.h"
#include "env.h"
#include "jniutils.h"

static jobject mprimitive2jvalue(morphine_coroutine_t U, struct env *env) {
    jobject value;
    if (mapi_is_type(U, "nil")) {
        value = env->value.nil.object;
    } else if (mapi_is_type(U, "integer")) {
        ml_integer integer = mapi_get_integer(U);
        value = J(env->jnienv, NewObject, env->value.integer.clazz, env->value.integer.constructor_id, integer);
    } else if (mapi_is_type(U, "decimal")) {
        ml_decimal decimal = mapi_get_decimal(U);
        value = J(env->jnienv, NewObject, env->value.decimal.clazz, env->value.decimal.constructor_id, decimal);
    } else if (mapi_is_type(U, "boolean")) {
        bool boolean = mapi_get_boolean(U);
        value = J(env->jnienv, NewObject, env->value.boolean.clazz, env->value.boolean.constructor_id, boolean);
    } else if (mapi_is_type(U, "string")) {
        const char *str = mapi_get_cstr(U);
        jstring string = J(env->jnienv, NewStringUTF, str);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        value = J(env->jnienv, NewObject, env->value.string.clazz, env->value.string.constructor_id, string);
        J(env->jnienv, DeleteLocalRef, string);
    } else {
        mapi_error(U, "cannot convert to jni value");
    }

    jniutils_check_exception(mapi_instance(U), env->jnienv);
    return value;
}

static jobject mvalue2jvalue(morphine_coroutine_t U, struct env *env) {
    if (mapi_is_type(U, "table")) {
        ml_size table_size = mapi_table_len(U);
        jsize size = jniutils_mlsize2jsize(U, table_size, NULL);
        jobject map = J(env->jnienv, NewObject, env->hash_map.clazz, env->hash_map.constructor_id, size);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        for (ml_size i = 0; i < table_size; i++) {
            mapi_table_idx_key(U, i);
            jobject key = mprimitive2jvalue(U, env);
            mapi_pop(U, 1);

            mapi_table_idx_get(U, i);
            jobject value = mprimitive2jvalue(U, env);
            mapi_pop(U, 1);

            jobject old = J(env->jnienv, CallObjectMethod, map, env->hash_map.put_id, key, value);
            jniutils_check_exception(mapi_instance(U), env->jnienv);

            J(env->jnienv, DeleteLocalRef, old);
            J(env->jnienv, DeleteLocalRef, key);
            J(env->jnienv, DeleteLocalRef, value);
        }

        jobject vector = J(env->jnienv, NewObject, env->value.table.clazz, env->value.table.constructor_id, map);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        J(env->jnienv, DeleteLocalRef, map);
        return vector;
    } else if (mapi_is_type(U, "vector")) {
        ml_size vector_len = mapi_vector_len(U);
        jsize size = jniutils_mlsize2jsize(U, vector_len, NULL);
        jobject list = J(env->jnienv, NewObject, env->array_list.clazz, env->array_list.constructor_id, size);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        for (ml_size i = 0; i < vector_len; i++) {
            mapi_vector_get(U, i);
            jobject value = mprimitive2jvalue(U, env);
            mapi_pop(U, 1);

            J(env->jnienv, CallBooleanMethod, list, env->array_list.add_id, value);
            jniutils_check_exception(mapi_instance(U), env->jnienv);

            J(env->jnienv, DeleteLocalRef, value);
        }

        jobject vector = J(env->jnienv, NewObject, env->value.vector.clazz, env->value.vector.constructor_id, list);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        J(env->jnienv, DeleteLocalRef, list);
        return vector;
    }

    return mprimitive2jvalue(U, env);
}

static void jvalue2mprimitive(morphine_coroutine_t U, struct env *env, jobject object) {
    if (J(env->jnienv, IsInstanceOf, object, env->value.nil.clazz)) {
        mapi_push_nil(U);
    } else if (J(env->jnienv, IsInstanceOf, object, env->value.integer.clazz)) {
        ml_integer integer = J(env->jnienv, GetLongField, object, env->value.integer.value_id);
        mapi_push_integer(U, integer);
    } else if (J(env->jnienv, IsInstanceOf, object, env->value.decimal.clazz)) {
        ml_decimal decimal = J(env->jnienv, GetDoubleField, object, env->value.decimal.value_id);
        mapi_push_decimal(U, decimal);
    } else if (J(env->jnienv, IsInstanceOf, object, env->value.boolean.clazz)) {
        jboolean boolean = J(env->jnienv, GetBooleanField, object, env->value.boolean.value_id);
        mapi_push_boolean(U, boolean);
    } else if (J(env->jnienv, IsInstanceOf, object, env->value.string.clazz)) {
        jstring string = J(env->jnienv, GetObjectField, object, env->value.string.value_id);
        const char *str = J(env->jnienv, GetStringUTFChars, string, NULL);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        mapi_push_string(U, str);

        J(env->jnienv, ReleaseStringUTFChars, string, str);
        J(env->jnienv, DeleteLocalRef, string);
    } else {
        mapi_error(U, "cannot convert from jni value");
    }
}

static void jvalue2mvalue(morphine_coroutine_t U, struct env *env, jobject object) {
    if (J(env->jnienv, IsInstanceOf, object, env->value.table.clazz)) {
        jobject map = J(env->jnienv, GetObjectField, object, env->value.table.value_id);
        jobject entries = J(env->jnienv, CallObjectMethod, map, env->hash_map.entry_set_id);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        jobject iterator = J(env->jnienv, CallObjectMethod, entries, env->set.iterator_id);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        J(env->jnienv, DeleteLocalRef, map);
        J(env->jnienv, DeleteLocalRef, entries);

        mapi_push_table(U);

        while (true) {
            jboolean has_next = J(env->jnienv, CallBooleanMethod, iterator, env->iterator.has_next_id);
            jniutils_check_exception(mapi_instance(U), env->jnienv);

            if (!has_next) {
                break;
            }

            jobject entry = J(env->jnienv, CallObjectMethod, iterator, env->iterator.next_id);
            jniutils_check_exception(mapi_instance(U), env->jnienv);

            jobject key = J(env->jnienv, CallObjectMethod, entry, env->map_entry.get_key_id);
            jniutils_check_exception(mapi_instance(U), env->jnienv);
            jvalue2mprimitive(U, env, key);

            jobject value = J(env->jnienv, CallObjectMethod, entry, env->map_entry.get_value_id);
            jniutils_check_exception(mapi_instance(U), env->jnienv);
            jvalue2mprimitive(U, env, value);

            mapi_table_set(U);

            J(env->jnienv, DeleteLocalRef, entry);
            J(env->jnienv, DeleteLocalRef, key);
            J(env->jnienv, DeleteLocalRef, value);
        }

        J(env->jnienv, DeleteLocalRef, iterator);
    } else if (J(env->jnienv, IsInstanceOf, object, env->value.vector.clazz)) {
        jobject list = J(env->jnienv, GetObjectField, object, env->value.vector.value_id);

        jsize size = J(env->jnienv, CallIntMethod, list, env->array_list.size_id);
        jniutils_check_exception(mapi_instance(U), env->jnienv);

        ml_size vector_len = jniutils_jsize2mlsize(U, size, NULL);
        mapi_push_vector(U, vector_len);

        for (ml_size i = 0; i < vector_len; i++) {
            jobject value = J(env->jnienv, CallObjectMethod, list, env->array_list.get_id, (jsize) i);
            jniutils_check_exception(mapi_instance(U), env->jnienv);
            jvalue2mprimitive(U, env, value);
            J(env->jnienv, DeleteLocalRef, value);

            mapi_vector_set(U, i);
        }

        J(env->jnienv, DeleteLocalRef, list);
    } else {
        jvalue2mprimitive(U, env, object);
    }
}

static void lib_call(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args_minimum(U, 1);
            struct env *env = mapi_instance_data(mapi_instance(U));

            J(env->jnienv, PushLocalFrame, 16);
            jniutils_check_exception(mapi_instance(U), env->jnienv);

            mapi_catchable(U, 1);

            jobjectArray array;
            {
                ml_size size = mapi_args(U) - 1;
                jsize args = jniutils_mlsize2jsize(U, size, NULL);
                array = J(env->jnienv, NewObjectArray, args, env->value.clazz, NULL);
                jniutils_check_exception(mapi_instance(U), env->jnienv);

                for (ml_size i = 0; i < size; i++) {
                    mapi_push_arg(U, i + 1);
                    jobject value = mvalue2jvalue(U, env);
                    mapi_pop(U, 1);

                    J(env->jnienv, SetObjectArrayElement, array, (jsize) i, value);
                    J(env->jnienv, DeleteLocalRef, value);
                }
            }

            jstring name;
            {
                mapi_push_arg(U, 0);
                const char *name_str = mapi_get_cstr(U);
                name = J(env->jnienv, NewStringUTF, name_str);
                mapi_pop(U, 1);
            }

            jobject result = J(env->jnienv, CallObjectMethod, env->callable.object, env->callable.call_id, name, array);
            jniutils_check_exception(mapi_instance(U), env->jnienv);

            J(env->jnienv, DeleteLocalRef, name);
            J(env->jnienv, DeleteLocalRef, array);

            jvalue2mvalue(U, env, result);

            J(env->jnienv, PopLocalFrame, NULL);

            mapi_uncatch(U);
            jniutils_check_exception(mapi_instance(U), env->jnienv);

            maux_nb_return();
        maux_nb_state(1)
            struct env *env = mapi_instance_data(mapi_instance(U));
            J(env->jnienv, PopLocalFrame, NULL);
            mapi_provide_error(U);
    maux_nb_end
}

static void lib_interrupted(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            struct env *env = mapi_instance_data(mapi_instance(U));
            jboolean is_interrupted =
                J(env->jnienv, CallStaticBooleanMethod, env->thread.clazz, env->thread.interrupted_id);
            jniutils_check_exception(mapi_instance(U), env->jnienv);
            mapi_push_boolean(U, is_interrupted == JNI_TRUE);
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
    MAUX_CONSTRUCT_FUNCTION("call", lib_call),
    MAUX_CONSTRUCT_FUNCTION("interrupted", lib_interrupted),
    MAUX_CONSTRUCT_FUNCTION("exit", lib_exit),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mjlib_jni(void) {
    return (morphine_library_t) {
        .name = "jni",
        .sharedkey = NULL,
        .init = library_init
    };
}
