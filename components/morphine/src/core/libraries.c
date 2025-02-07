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
#include "morphine/gc/safe.h"
#include "morphine/object/native.h"
#include "morphine/object/coroutine.h"
#include "morphine/misc/sharedstorage.h"

struct libraries librariesI_prototype(void) {
    return (struct libraries) {
        .list = NULL
    };
}

static void library_free(morphine_instance_t I, struct library *library) {
    allocI_free(I, library);
}

void librariesI_free(morphine_instance_t I, struct libraries *libraries) {
    struct library *library = libraries->list;
    while (library != NULL) {
        struct library *prev = library->prev;
        library_free(I, library);

        library = prev;
    }
}

void librariesI_load(morphine_instance_t I, morphine_library_t library) {
    {
        struct library *current = I->libraries.list;
        while (current != NULL) {
            if (stringI_cstr_compare(current->name, library.name) == 0) {
                throwI_error(I, "library already loaded");
            }

            current = current->prev;
        }
    }

    gcI_safe_enter(I);
    struct string *name = gcI_safe_obj(I, string, stringI_create(I, library.name));

    if (library.sharedkey != NULL && !sharedstorageI_define(I, library.sharedkey)) {
        throwI_error(I, "library sharedstorage conflict");
    }

    struct library *result = allocI_uni(I, NULL, sizeof(struct library));
    (*result) = (struct library) {
        .init = library.init,
        .name = name,
        .table = NULL,
        .prev = I->libraries.list
    };

    I->libraries.list = result;

    gcI_safe_exit(I);
}

static struct table *init_library(morphine_coroutine_t U, mfunc_native_t init) {
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
    struct library *library = U->I->libraries.list;
    while (library != NULL) {
        if (stringI_cstr_compare(library->name, name) == 0) {
            break;
        }

        library = library->prev;
    }

    if (library == NULL) {
        throwI_error(U->I, "library not found");
    }

    if (library->table == NULL) {
        library->table = init_library(U, library->init);
    }

    return valueI_object(library->table);
}
