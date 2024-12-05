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
#include "morphine/object/coroutine.h"
#include "morphine/misc/sharedstorage.h"

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

void librariesI_load(morphine_instance_t I, morphine_library_t library) {
    for (size_t i = 0; i < I->libraries.size; i++) {
        if (stringI_cstr_compare(I, I->libraries.array[i].name, library.name) == 0) {
            throwI_error(I, "library already loaded");
        }
    }

    if (unlikely(I->libraries.allocated == I->libraries.size)) {
        overflow_add(I->libraries.allocated, MPARAM_LIBRARIES_GROW, SIZE_MAX) {
            throwI_error(I, "library limit exceeded");
        }

        I->libraries.array = allocI_vec(
            I,
            I->libraries.array,
            I->libraries.allocated + MPARAM_LIBRARIES_GROW,
            sizeof(struct library_instance)
        );

        for (size_t i = 0; i < MPARAM_LIBRARIES_GROW; i++) {
            I->libraries.array[i + I->libraries.allocated] = (struct library_instance) {
                .name = NULL,
                .init = NULL,
                .table = NULL
            };
        }

        I->libraries.allocated += MPARAM_LIBRARIES_GROW;
    }

    gcI_safe_enter(I);
    struct string *name = gcI_safe_obj(I, string, stringI_create(I, library.name));

    if (library.sharedkey != NULL && !sharedstorageI_define(I, library.sharedkey)) {
        throwI_error(I, "library sharedkey conflict");
    }

    I->libraries.array[I->libraries.size] = (struct library_instance) {
        .name = name,
        .init = library.init,
        .table = NULL
    };

    I->libraries.size++;

    gcI_safe_exit(I);
}

static struct table *init_library(morphine_coroutine_t U, morphine_library_init_t init) {
    size_t space_size = stackI_space(U);

    init(U);

    gcI_safe_enter(U->I);
    struct table *table = gcI_safe_obj(U->I, table, valueI_as_table_or_error(U->I, stackI_peek(U, 0)));

    stackI_pop(U, 1);
    if (stackI_space(U) != space_size) {
        throwI_error(U->I, "library initialization corrupted");
    }

    gcI_safe_exit(U->I);

    return table;
}

struct value librariesI_get(morphine_coroutine_t U, const char *name) {
    struct library_instance *library = NULL;
    for (size_t i = 0; i < U->I->libraries.size; i++) {
        struct library_instance *lib = U->I->libraries.array + i;
        if (stringI_cstr_compare(U->I, lib->name, name) == 0) {
            library = lib;
            break;
        }
    }

    if (library == NULL) {
        throwI_error(U->I, "library not found");
    }

    if (library->table == NULL) {
        library->table = init_library(U, library->init);
    }

    return valueI_object(library->table);
}
