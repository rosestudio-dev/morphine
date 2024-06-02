//
// Created by why-iskra on 22.05.2024.
//

#include <memory.h>
#include "morphine/compiler/strtable.h"
#include "config.h"

#define MORPHINE_TYPE "strtable"

struct entry {
    char *string;
    size_t size;
};

struct strtable {
    size_t size;
    size_t used;
    struct entry *entries;
};

static void strtable_free(morphine_instance_t I, void *p) {
    struct strtable *T = p;

    for (size_t i = 0; i < T->used; i++) {
        mapi_allocator_free(I, T->entries[i].string);
    }

    mapi_allocator_free(I, T->entries);
}

static struct strtable *get_strtable(morphine_coroutine_t U) {
    if (strcmp(mapi_userdata_type(U), MORPHINE_TYPE) == 0) {
        return mapi_userdata_pointer(U);
    } else {
        mapi_error(U, "expected "MORPHINE_TYPE);
    }
}

void strtable(morphine_coroutine_t U) {
    struct strtable *T = mapi_push_userdata(U, MORPHINE_TYPE, sizeof(struct strtable));

    *T = (struct strtable) {
        .used = 0,
        .size = 0,
        .entries = NULL
    };

    mapi_userdata_set_free(U, strtable_free);
}

strtable_index_t strtable_record(morphine_coroutine_t U, const char *str, size_t size) {
    struct strtable *T = get_strtable(U);

    for (size_t i = 0; i < T->used; i++) {
        struct entry entry = T->entries[i];

        if (entry.size != size) {
            continue;
        }

        if (memcmp(entry.string, str, size) == 0) {
            return i;
        }
    }

    if (T->used == T->size) {
        if (T->size >= STRTABLE_LIMIT_ENTRIES) {
            mapi_error(U, "strtable too big");
        }

        T->entries = mapi_allocator_vec(
            mapi_instance(U),
            T->entries,
            T->size + STRTABLE_EXPANSION_FACTOR,
            sizeof(struct strtable_entry)
        );

        T->size += STRTABLE_EXPANSION_FACTOR;
    }

    char *buffer = mapi_allocator_uni(mapi_instance(U), NULL, size);
    struct entry entry = {
        .string = buffer,
        .size = size
    };

    memcpy(buffer, str, size);

    T->entries[T->used] = entry;
    T->used++;

    return T->used - 1;
}

bool strtable_has(morphine_coroutine_t U, strtable_index_t index) {
    struct strtable *T = get_strtable(U);

    if (index >= T->used) {
        return false;
    }

    return true;
}

struct strtable_entry strtable_get(morphine_coroutine_t U, strtable_index_t index) {
    struct strtable *T = get_strtable(U);

    if (index >= T->used) {
        mapi_error(U, "string not found");
    }

    struct entry entry = T->entries[index];
    return (struct strtable_entry) {
        .string = entry.string,
        .size = entry.size
    };
}
