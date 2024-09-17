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

static struct string *access(
    morphine_instance_t I,
    struct table **dest_table,
    const char *name
) {
    size_t name_len = strlen(name);
    size_t last = 0;
    for (size_t i = 0; i < name_len; i++) {
        if (name[i] != '.') {
            continue;
        }

        size_t part_size = i - last;

        if (part_size == 0) {
            throwI_error(I, "empty sub-library name");
        }

        struct string *key = stringI_createn(I, part_size, name + last);
        size_t rollback = gcI_safe_obj(I, objectI_cast(key));

        bool has = false;
        struct value dest_table_value = tableI_get(I, *dest_table, valueI_object(key), &has);

        if (!has) {
            struct table *nested_table = tableI_create(I);
            gcI_safe_obj(I, objectI_cast(nested_table));
            dest_table_value = valueI_object(nested_table);

            tableI_set(I, *dest_table, valueI_object(key), dest_table_value);
        }

        *dest_table = valueI_as_table_or_error(I, dest_table_value);
        last = i + 1;
        gcI_reset_safe(I, rollback);
    }

    size_t part_size = name_len - last;

    if (part_size == 0) {
        throwI_error(I, "empty library entry name");
    }

    return stringI_createn(I, part_size, name + last);
}

static void constructor_insert(
    morphine_instance_t I,
    struct table *root_table,
    const char *name,
    struct value value
) {
    struct table *dest_table = root_table;
    struct string *key = access(I, &dest_table, name);
    size_t rollback_key = gcI_safe_obj(I, objectI_cast(key));

    tableI_set(I, dest_table, valueI_object(key), value);

    gcI_reset_safe(I, rollback_key);
}

static struct value constructor_get(
    morphine_instance_t I,
    struct table *root_table,
    const char *name
) {
    struct table *dest_table = root_table;
    struct string *key = access(I, &dest_table, name);
    size_t rollback_key = gcI_safe_obj(I, objectI_cast(key));

    bool has = false;
    struct value value = tableI_get(I, dest_table, valueI_object(key), &has);

    gcI_reset_safe(I, rollback_key);

    if (!has) {
        throwI_errorf(I, "library entry not found");
    }

    return value;
}

static struct table *construct(morphine_instance_t I, morphine_library_t *L) {
    struct table *result = tableI_create(I);
    size_t rollback = gcI_safe_obj(I, objectI_cast(result));

    for (morphine_library_function_t *entry = L->functions; entry != NULL && entry->name != NULL; entry++) {
        struct string *name = stringI_createf(I, "%s.%s", L->name, entry->name);
        size_t rollback_name = gcI_safe_obj(I, objectI_cast(name));

        struct native *value = nativeI_create(I, name->chars, entry->function);
        gcI_safe_obj(I, objectI_cast(value));

        constructor_insert(I, result, entry->name, valueI_object(value));

        gcI_reset_safe(I, rollback_name);
    }

    for (morphine_library_string_t *entry = L->strings; entry != NULL && entry->name != NULL; entry++) {
        struct string *value = stringI_create(I, entry->string);
        size_t rollback_value = gcI_safe_obj(I, objectI_cast(value));

        constructor_insert(I, result, entry->name, valueI_object(value));

        gcI_reset_safe(I, rollback_value);
    }

    for (morphine_library_integer_t *entry = L->integers; entry != NULL && entry->name != NULL; entry++) {
        constructor_insert(I, result, entry->name, valueI_integer(entry->integer));
    }

    for (morphine_library_decimal_t *entry = L->decimals; entry != NULL && entry->name != NULL; entry++) {
        constructor_insert(I, result, entry->name, valueI_decimal(entry->decimal));
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
            entry->params.init,
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

struct value librariesI_get(morphine_instance_t I, const char *name, bool reload) {
    size_t name_len = strlen(name);
    size_t size = 0;
    for (; size < name_len; size++) {
        if (name[size] == '.') {
            break;
        }
    }

    struct library *library = NULL;
    for (size_t i = 0; i < I->libraries.size; i++) {
        struct library *lib = I->libraries.array + i;

        if (lib->L == NULL) {
            continue;
        }

        if (strlen(lib->L->name) == size &&
            memcmp(lib->L->name, name, size * sizeof(char)) == 0) {
            library = lib;
            break;
        }
    }

    if (library == NULL) {
        throwI_error(I, "library not found");
    }

    if (reload || library->table == NULL) {
        library->table = construct(I, library->L);
    }

    if (size == name_len) {
        return valueI_object(library->table);
    }

    return constructor_get(I, library->table, name + size + 1);
}
