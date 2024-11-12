//
// Created by why-iskra on 12.11.2024.
//

#include "jnilib.h"
#include "env.h"
#include "jniutils.h"

#define SHAREDKEY ("morphine-misc-jnilib")

#define SIG(c) "L"c";"

#define CLASS_RECEIVER                "ru/why/morphine/jni/Morphine$Receiver"
#define CLASS_VALUE                   "ru/why/morphine/jni/Morphine$Value"
#define CLASS_VALUE_TABLE             "ru/why/morphine/jni/Morphine$Value$MLTable"
#define CLASS_VALUE_VECTOR            "ru/why/morphine/jni/Morphine$Value$MLVector"
#define CLASS_VALUE_PRIMITIVE_NIL     "ru/why/morphine/jni/Morphine$Value$Primitive$MLNil"
#define CLASS_VALUE_PRIMITIVE_INTEGER "ru/why/morphine/jni/Morphine$Value$Primitive$MLInteger"
#define CLASS_VALUE_PRIMITIVE_DECIMAL "ru/why/morphine/jni/Morphine$Value$Primitive$MLDecimal"
#define CLASS_VALUE_PRIMITIVE_BOOLEAN "ru/why/morphine/jni/Morphine$Value$Primitive$MLBoolean"
#define CLASS_VALUE_PRIMITIVE_STRING  "ru/why/morphine/jni/Morphine$Value$Primitive$MLString"

struct jnidata {
    JNIEnv *jnienv;
    jobject this;

    jobject receiver;
    jmethodID send_id;
    jmethodID receive_id;
    jmethodID waitreceive_id;
};

static jobject to_primitive(morphine_coroutine_t U, struct jnidata *D) {
    if (mapi_is(U, "nil")) {
        mapi_pop(U, 1);

        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_NIL);
        jfieldID field = J(D->jnienv, GetStaticFieldID, class, "INSTANCE", SIG(CLASS_VALUE_PRIMITIVE_NIL));
        jobject object = J(D->jnienv, GetStaticObjectField, class, field);

        J(D->jnienv, DeleteLocalRef, class);
        return object;
    }

    if (mapi_is(U, "integer")) {
        ml_integer integer = mapi_get_integer(U);
        mapi_pop(U, 1);

        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_INTEGER);
        jmethodID method = J(D->jnienv, GetMethodID, class, "<init>", "(J)V");
        jobject object = J(D->jnienv, NewObject, class, method, (jlong) integer);

        J(D->jnienv, DeleteLocalRef, class);
        return object;
    }

    if (mapi_is(U, "decimal")) {
        ml_decimal decimal = mapi_get_decimal(U);
        mapi_pop(U, 1);

        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_DECIMAL);
        jmethodID method = J(D->jnienv, GetMethodID, class, "<init>", "(D)V");
        jobject object = J(D->jnienv, NewObject, class, method, (jdouble) decimal);

        J(D->jnienv, DeleteLocalRef, class);
        return object;
    }

    if (mapi_is(U, "boolean")) {
        bool boolean = mapi_get_boolean(U);
        mapi_pop(U, 1);

        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_BOOLEAN);
        jmethodID method = J(D->jnienv, GetMethodID, class, "<init>", "(Z)V");
        jobject object = J(D->jnienv, NewObject, class, method, (jboolean) boolean);

        J(D->jnienv, DeleteLocalRef, class);
        return object;

    }

    if (mapi_is(U, "string")) {
        jstring string = jniutils_mlstring2jstring(U, D->jnienv);
        mapi_pop(U, 1);

        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_STRING);
        jmethodID method = J(D->jnienv, GetMethodID, class, "<init>", "(Ljava/lang/String;)V");
        jobject object = J(D->jnienv, NewObject, class, method, string);

        J(D->jnienv, DeleteLocalRef, class);
        J(D->jnienv, DeleteLocalRef, string);
        return object;
    }

    return NULL;
}

static jobject to_value(morphine_coroutine_t U, struct jnidata *D) {
    if (mapi_is(U, "vector")) {
        ml_size size = mapi_vector_len(U);

        jclass list_class = J(D->jnienv, FindClass, "java/util/ArrayList");
        jmethodID list_method = J(D->jnienv, GetMethodID, list_class, "<init>", "()V");
        jmethodID list_method_add = J(
            D->jnienv,
            GetMethodID,
            list_class,
            "add",
            "(Ljava/lang/Object;)Z"
        );

        jobject list_object = J(
            D->jnienv,
            NewObject,
            list_class,
            list_method
        );

        for (ml_size i = 0; i < size; i++) {
            mapi_vector_get(U, i);
            jobject primitive = to_primitive(U, D);
            if (primitive == NULL) {
                J(D->jnienv, DeleteLocalRef, list_class);
                J(D->jnienv, DeleteLocalRef, list_object);

                goto error;
            }

            J(D->jnienv, CallBooleanMethod, list_object, list_method_add, primitive);
            J(D->jnienv, DeleteLocalRef, primitive);
        }

        mapi_pop(U, 1);

        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_VECTOR);
        jmethodID method = J(D->jnienv, GetMethodID, class, "<init>", "(Ljava/util/List;)V");
        jobject object = J(D->jnienv, NewObject, class, method, list_object);

        J(D->jnienv, DeleteLocalRef, class);
        J(D->jnienv, DeleteLocalRef, list_class);
        J(D->jnienv, DeleteLocalRef, list_object);

        return object;
    }

    if (mapi_is(U, "table")) {
        ml_size size = mapi_table_len(U);

        jclass map_class = J(D->jnienv, FindClass, "java/util/HashMap");
        jmethodID map_method = J(D->jnienv, GetMethodID, map_class, "<init>", "()V");
        jmethodID map_method_set = J(
            D->jnienv,
            GetMethodID,
            map_class,
            "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"
        );

        bool success = false;
        jsize mapsize = jniutils_mlsize2jsize(U, size, &success);
        if (!success) {
            J(D->jnienv, DeleteLocalRef, map_class);
            goto error;
        }

        jobject map_object = J(
            D->jnienv,
            NewObject,
            map_class,
            map_method,
            mapsize
        );

        for (ml_size i = 0; i < size; i++) {
            mapi_table_idx_key(U, i);
            jobject primitive_key = to_primitive(U, D);
            if (primitive_key == NULL) {
                J(D->jnienv, DeleteLocalRef, map_class);
                J(D->jnienv, DeleteLocalRef, map_object);

                goto error;
            }

            mapi_table_idx_get(U, i);
            jobject primitive_value = to_primitive(U, D);
            if (primitive_value == NULL) {
                J(D->jnienv, DeleteLocalRef, map_class);
                J(D->jnienv, DeleteLocalRef, map_object);

                goto error;
            }

            jobject result = J(
                D->jnienv,
                CallObjectMethod,
                map_object,
                map_method_set,
                primitive_key,
                primitive_value
            );

            J(D->jnienv, DeleteLocalRef, primitive_key);
            J(D->jnienv, DeleteLocalRef, primitive_value);
            J(D->jnienv, DeleteLocalRef, result);
        }

        mapi_pop(U, 1);

        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_TABLE);
        jmethodID method = J(D->jnienv, GetMethodID, class, "<init>", "(Ljava/util/Map;)V");
        jobject object = J(D->jnienv, NewObject, class, method, map_object);

        J(D->jnienv, DeleteLocalRef, class);
        J(D->jnienv, DeleteLocalRef, map_class);
        J(D->jnienv, DeleteLocalRef, map_object);

        return object;
    }

    jobject primitive = to_primitive(U, D);
    if (primitive == NULL) {
        goto error;
    }

    return primitive;
error:
    mapi_error(U, "unsupported value");
}

static bool jni_is_instance(struct jnidata *D, jobject object, const char *type) {
    jclass class = J(D->jnienv, FindClass, type);
    jboolean result = J(D->jnienv, IsInstanceOf, object, class);
    J(D->jnienv, DeleteLocalRef, class);
    return result;
}

static bool from_primitive(morphine_coroutine_t U, struct jnidata *D, jobject object) {
    if (jni_is_instance(D, object, CLASS_VALUE_PRIMITIVE_NIL)) {
        mapi_push_nil(U);
        return true;
    }

    if (jni_is_instance(D, object, CLASS_VALUE_PRIMITIVE_INTEGER)) {
        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_INTEGER);
        jfieldID field = J(D->jnienv, GetFieldID, class, "value", "J");
        jlong value = J(D->jnienv, GetLongField, object, field);
        J(D->jnienv, DeleteLocalRef, class);

        mapi_push_integer(U, value);
        return true;
    }

    if (jni_is_instance(D, object, CLASS_VALUE_PRIMITIVE_DECIMAL)) {
        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_DECIMAL);
        jfieldID field = J(D->jnienv, GetFieldID, class, "value", "D");
        jdouble value = J(D->jnienv, GetDoubleField, object, field);
        J(D->jnienv, DeleteLocalRef, class);

        mapi_push_decimal(U, value);
        return true;
    }

    if (jni_is_instance(D, object, CLASS_VALUE_PRIMITIVE_BOOLEAN)) {
        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_BOOLEAN);
        jfieldID field = J(D->jnienv, GetFieldID, class, "value", "Z");
        jboolean value = J(D->jnienv, GetBooleanField, object, field);
        J(D->jnienv, DeleteLocalRef, class);

        mapi_push_boolean(U, value);
        return true;
    }

    if (jni_is_instance(D, object, CLASS_VALUE_PRIMITIVE_STRING)) {
        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_PRIMITIVE_STRING);
        jfieldID field = J(D->jnienv, GetFieldID, class, "value", "Ljava/lang/String;");
        jstring value = J(D->jnienv, GetObjectField, object, field);

        bool error = false;
        jniutils_jstring2mlstring(U, D->jnienv, value, &error);

        J(D->jnienv, DeleteLocalRef, class);
        J(D->jnienv, DeleteLocalRef, value);

        return !error;
    }

    return false;
}

static bool from_value(morphine_coroutine_t U, struct jnidata *D, jobject object) {
    if (jni_is_instance(D, object, CLASS_VALUE_VECTOR)) {
        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_VECTOR);
        jfieldID field = J(D->jnienv, GetFieldID, class, "value", "Ljava/util/List;");

        jobject list_object = J(D->jnienv, GetObjectField, object, field);
        jclass list_class = J(D->jnienv, GetObjectClass, list_object);
        jmethodID list_size = J(D->jnienv, GetMethodID, list_class, "size", "()I");
        jmethodID list_get = J(D->jnienv, GetMethodID, list_class, "get", "(I)Ljava/lang/Object;");

        jint size = J(D->jnienv, CallIntMethod, list_object, list_size);
        bool error = false;
        ml_size vector_size = jniutils_jsize2mlsize(U, size, &error);

        if (error) {
            J(D->jnienv, DeleteLocalRef, class);
            J(D->jnienv, DeleteLocalRef, list_class);
            J(D->jnienv, DeleteLocalRef, list_object);
            return false;
        }

        mapi_push_vector(U, vector_size);
        for (jint i = 0; i < size; i++) {
            jobject result = J(D->jnienv, CallObjectMethod, list_object, list_get, i);
            bool success = from_primitive(U, D, result);
            J(D->jnienv, DeleteLocalRef, object);

            if (!success) {
                J(D->jnienv, DeleteLocalRef, class);
                J(D->jnienv, DeleteLocalRef, list_class);
                J(D->jnienv, DeleteLocalRef, list_object);
                return false;
            }

            mapi_vector_set(U, (ml_size) i);
        }

        J(D->jnienv, DeleteLocalRef, class);
        J(D->jnienv, DeleteLocalRef, list_class);
        J(D->jnienv, DeleteLocalRef, list_object);
        return true;
    }

    if (jni_is_instance(D, object, CLASS_VALUE_TABLE)) {
        jclass class = J(D->jnienv, FindClass, CLASS_VALUE_TABLE);
        jfieldID field = J(D->jnienv, GetFieldID, class, "value", "Ljava/util/Map;");

        jobject map_object = J(D->jnienv, GetObjectField, object, field);
        jclass map_class = J(D->jnienv, GetObjectClass, map_object);
        jmethodID map_entries = J(D->jnienv, GetMethodID, map_class, "entrySet", "()Ljava/util/Set;");

        jobject entries_object = J(D->jnienv, CallObjectMethod, map_object, map_entries);
        jclass entries_class = J(D->jnienv, GetObjectClass, entries_object);
        jmethodID entries_iterator =
            J(D->jnienv, GetMethodID, entries_class, "iterator", "()Ljava/util/Iterator;");

        jobject iterator_object = J(D->jnienv, CallObjectMethod, entries_object, entries_iterator);
        jclass iterator_class = J(D->jnienv, GetObjectClass, iterator_object);
        jmethodID iterator_has = J(D->jnienv, GetMethodID, iterator_class, "hasNext", "()Z");
        jmethodID iterator_next = J(D->jnienv, GetMethodID, iterator_class, "next", "()Ljava/lang/Object;");

        jclass entry_class = J(D->jnienv, FindClass, "java/util/Map$Entry");
        jmethodID entry_key = J(D->jnienv, GetMethodID, entry_class, "getKey", "()Ljava/lang/Object;");
        jmethodID entry_value = J(D->jnienv, GetMethodID, entry_class, "getValue", "()Ljava/lang/Object;");

        J(D->jnienv, DeleteLocalRef, class);
        J(D->jnienv, DeleteLocalRef, map_object);
        J(D->jnienv, DeleteLocalRef, map_class);
        J(D->jnienv, DeleteLocalRef, entries_object);
        J(D->jnienv, DeleteLocalRef, entries_class);
        J(D->jnienv, DeleteLocalRef, iterator_class);
        J(D->jnienv, DeleteLocalRef, entry_class);

        mapi_push_table(U);
        while (true) {
            bool has = J(D->jnienv, CallBooleanMethod, iterator_object, iterator_has);
            if (!has) {
                break;
            }

            jobject entry = J(D->jnienv, CallObjectMethod, iterator_object, iterator_next);
            jobject key = J(D->jnienv, CallObjectMethod, entry, entry_key);
            jobject value = J(D->jnienv, CallObjectMethod, entry, entry_value);

            bool success_key = from_primitive(U, D, key);
            bool success_value = from_primitive(U, D, value);

            J(D->jnienv, DeleteLocalRef, key);
            J(D->jnienv, DeleteLocalRef, value);
            J(D->jnienv, DeleteLocalRef, entry);

            if (!success_key || !success_value) {
                J(D->jnienv, DeleteLocalRef, iterator_object);
                return false;
            }

            mapi_table_set(U);
        }

        return true;
    }

    return from_primitive(U, D, object);
}

static void lib_send(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            maux_sharedstorage_get(U, SHAREDKEY, "data");
            struct jnidata *D = mapi_userdata_pointer(U, NULL);

            if (D->receiver == NULL) {
                maux_nb_leave();
            }

            mapi_push_arg(U, 0);
            jobject object = to_value(U, D);

            J(D->jnienv, CallVoidMethod, D->receiver, D->send_id, object);
            J(D->jnienv, DeleteLocalRef, object);

            maux_nb_leave();
    maux_nb_end
}

static void lib_receive(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);

            maux_sharedstorage_get(U, SHAREDKEY, "data");
            struct jnidata *D = mapi_userdata_pointer(U, NULL);

            jobject object = J(D->jnienv, CallObjectMethod, D->this, D->receive_id);
            bool success = from_value(U, D, object);
            J(D->jnienv, DeleteLocalRef, object);

            if (!success) {
                mapi_error(U, "unsupported value");
            }

            maux_nb_return();
    maux_nb_end
}

static void lib_waitreceive(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);

            maux_sharedstorage_get(U, SHAREDKEY, "data");
            struct jnidata *D = mapi_userdata_pointer(U, NULL);

            jobject object = J(D->jnienv, CallObjectMethod, D->this, D->waitreceive_id);
            bool success = from_value(U, D, object);
            J(D->jnienv, DeleteLocalRef, object);

            if (!success) {
                mapi_error(U, "unsupported value");
            }

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
    MAUX_CONSTRUCT_FUNCTION("send", lib_send),
    MAUX_CONSTRUCT_FUNCTION("receive", lib_receive),
    MAUX_CONSTRUCT_FUNCTION("waitreceive", lib_waitreceive),
    MAUX_CONSTRUCT_FUNCTION("exit", lib_exit),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    struct env *env = mapi_instance_data(mapi_instance(U));

    jclass this_class = J(env->jnienv, GetObjectClass, env->this);
    jmethodID receive_id = J(env->jnienv, GetMethodID, this_class, "receive", "()"SIG(CLASS_VALUE));
    jmethodID waitreceive_id = J(env->jnienv, GetMethodID, this_class, "waitReceive", "()"SIG(CLASS_VALUE));
    jfieldID receiver_id = J(env->jnienv, GetFieldID, this_class, "receiver", SIG(CLASS_RECEIVER));

    jobject receiver = J(env->jnienv, GetObjectField, env->this, receiver_id);

    jmethodID send_id = 0;
    if (receiver != NULL) {
        jclass receiver_class = J(env->jnienv, GetObjectClass, receiver);
        send_id = J(env->jnienv, GetMethodID, receiver_class, "receive", "("SIG(CLASS_VALUE)")V");
        J(env->jnienv, DeleteLocalRef, receiver_class);
    }

    J(env->jnienv, DeleteLocalRef, this_class);

    struct jnidata *D = mapi_push_userdata_uni(U, sizeof(struct jnidata));
    (*D) = (struct jnidata) {
        .jnienv = env->jnienv,
        .this = env->this,
        .receiver = receiver,
        .send_id = send_id,
        .receive_id = receive_id,
        .waitreceive_id = waitreceive_id
    };

    maux_sharedstorage_set(U, SHAREDKEY, "data");
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mjlib_jni(void) {
    return (morphine_library_t) {
        .name = "jni",
        .sharedkey = SHAREDKEY,
        .init = library_init
    };
}
