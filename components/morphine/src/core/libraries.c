//
// Created by why-iskra on 17.06.2024.
//

#include <string.h>
#include "morphine/core/libraries.h"
#include "morphine/core/instance.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/utils/overflow.h"
#include "morphine/gc/allocator.h"
#include "morphine/params.h"
#include "morphine/gc/safe.h"
#include "morphine/object/native.h"

static void grow(morphine_instance_t I) {
    if (unlikely(I->libraries.allocated == I->libraries.size)) {
        overflow_add(I->libraries.allocated, MPARAM_LIBRARIES_GROW, SIZE_MAX) {
            throwI_error(I, "library limit exceeded");
        }

        I->libraries.array = allocI_vec(
            I, I->libraries.array, I->libraries.allocated + MPARAM_LIBRARIES_GROW, sizeof(struct library)
        );

        for (size_t i = 0; i < MPARAM_LIBRARIES_GROW; i++) {
            I->libraries.array[i + I->libraries.allocated] = (struct library) {
                .L = NULL,
                .table = NULL
            };
        }

        I->libraries.allocated += MPARAM_LIBRARIES_GROW;
    }
}

static struct table *construct(morphine_instance_t I, morphine_library_t *L) {
    struct table *result = tableI_create(I);
    size_t rollback = gcI_safe_obj(I, objectI_cast(result));

    for (morphine_library_function_t *entry = L->functions; entry != NULL && entry->name != NULL; entry++) {
        struct string *key = stringI_create(I, entry->name);
        size_t rollback_key = gcI_safe_obj(I, objectI_cast(key));

        struct string *name = stringI_createf(I, "%s.%s", L->name, entry->name);
        gcI_safe_obj(I, objectI_cast(name));

        struct native *value = nativeI_create(I, name->chars, entry->function);
        gcI_safe_obj(I, objectI_cast(value));

        tableI_set(I, result, valueI_object(key), valueI_object(value));

        gcI_reset_safe(I, rollback_key);
    }

    for (morphine_library_string_t *entry = L->strings; entry != NULL && entry->name != NULL; entry++) {
        struct string *key = stringI_create(I, entry->name);
        size_t rollback_key = gcI_safe_obj(I, objectI_cast(key));

        struct string *value = stringI_create(I, entry->string);
        gcI_safe_obj(I, objectI_cast(value));

        tableI_set(I, result, valueI_object(key), valueI_object(value));

        gcI_reset_safe(I, rollback_key);
    }

    for (morphine_library_integer_t *entry = L->integers; entry != NULL && entry->name != NULL; entry++) {
        struct string *key = stringI_create(I, entry->name);
        size_t rollback_key = gcI_safe_obj(I, objectI_cast(key));

        tableI_set(I, result, valueI_object(key), valueI_integer(entry->integer));

        gcI_reset_safe(I, rollback_key);
    }

    for (morphine_library_decimal_t *entry = L->decimals; entry != NULL && entry->name != NULL; entry++) {
        struct string *key = stringI_create(I, entry->name);
        size_t rollback_key = gcI_safe_obj(I, objectI_cast(key));

        tableI_set(I, result, valueI_object(key), valueI_decimal(entry->decimal));

        gcI_reset_safe(I, rollback_key);
    }

    tableI_mode_fixed(I, result, true);
    tableI_mode_mutable(I, result, false);
    tableI_mode_lock_metatable(I, result);
    tableI_mode_lock(I, result);

    gcI_reset_safe(I, rollback);
    return result;
}

static void library_init(morphine_instance_t I, morphine_library_t *L) {
    for (morphine_library_type_t *entry = L->types; entry != NULL && entry->name != NULL; entry++) {
        usertypeI_declare(
            I,
            entry->name,
            entry->params.allocate,
            entry->params.free,
            entry->params.require_metatable
        );
    }
}

struct libraries librariesI_prototype(void) {
    return (struct libraries) {
        .allocated = 0,
        .size = 0,
        .array = NULL
    };
}

void librariesI_free(morphine_instance_t I, struct libraries *libraries) {
    allocI_free(I, libraries->array);
    *libraries = librariesI_prototype();
}

void librariesI_shrink(morphine_instance_t I) {
    if (unlikely(I->libraries.allocated > I->libraries.size)) {
        I->libraries.array = allocI_vec(
            I, I->libraries.array, I->libraries.size, sizeof(struct library)
        );

        I->libraries.allocated = I->libraries.size;
    }
}

void librariesI_load(morphine_instance_t I, morphine_library_t *L) {
    if (L == NULL) {
        throwI_error(I, "library is null");
    }

    for (size_t i = 0; i < I->libraries.size; i++) {
        if (I->libraries.array[i].L == L) {
            return;
        }
    }

    for (size_t i = 0; i < I->libraries.size; i++) {
        if (I->libraries.array[i].L == NULL) {
            I->libraries.array[i] = (struct library) {
                .L = L,
                .table = NULL
            };

            return;
        }
    }

    grow(I);

    I->libraries.array[I->libraries.size] = (struct library) {
        .L = L,
        .table = NULL
    };

    I->libraries.size++;

    library_init(I, L);
}

struct table *librariesI_get(morphine_instance_t I, const char *name, bool reload) {
    struct library *library = NULL;
    for (size_t i = 0; i < I->libraries.size; i++) {
        struct library *lib = I->libraries.array + i;

        if (lib->L == NULL) {
            continue;
        }

        if (strcmp(lib->L->name, name) == 0) {
            library = lib;
            break;
        }
    }

    if (library == NULL) {
        throwI_errorf(I, "library '%s' not found", name);
    }

    if (reload || library->table == NULL) {
        library->table = construct(I, library->L);
    }

    return library->table;
}
