//
// Created by why-iskra on 22.05.2024.
//

#include <memory.h>
#include "morphinec/strtable.h"

#define LIMIT_ENTRIES    131072
#define EXPANSION_FACTOR 32

struct entry {
    char *string;
    size_t size;
};

struct mc_strtable {
    ml_size size;
    ml_size used;
    struct entry *entries;
};

static void strtable_userdata_init(morphine_instance_t I, void *data) {
    (void) I;

    struct mc_strtable *T = data;
    (*T) = (struct mc_strtable) {
        .used = 0,
        .size = 0,
        .entries = NULL
    };
}

static void strtable_userdata_free(morphine_instance_t I, void *data) {
    struct mc_strtable *T = data;

    for (size_t i = 0; i < T->used; i++) {
        mapi_allocator_free(I, T->entries[i].string);
    }

    mapi_allocator_free(I, T->entries);
}

MORPHINE_API struct mc_strtable *mcapi_push_strtable(morphine_coroutine_t U) {
    mapi_type_declare(
        mapi_instance(U),
        MC_STRTABLE_USERDATA_TYPE,
        sizeof(struct mc_strtable),
        strtable_userdata_init,
        strtable_userdata_free,
        false
    );

    return mapi_push_userdata(U, MC_STRTABLE_USERDATA_TYPE);
}

MORPHINE_API struct mc_strtable *mcapi_get_strtable(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, MC_STRTABLE_USERDATA_TYPE);
}

MORPHINE_API bool mcapi_strtable_has(struct mc_strtable *T, mc_strtable_index_t index) {
    if (index >= T->used) {
        return false;
    }

    return true;
}

MORPHINE_API mc_strtable_index_t mcapi_strtable_record(
    morphine_coroutine_t U,
    struct mc_strtable *T,
    const char *str,
    size_t size
) {
    for (ml_size i = 0; i < T->used; i++) {
        struct entry entry = T->entries[i];

        if (entry.size != size) {
            continue;
        }

        if (memcmp(entry.string, str, size * sizeof(char)) == 0) {
            return i;
        }
    }

    if (T->used == T->size) {
        if (T->size >= LIMIT_ENTRIES) {
            mapi_error(U, "mcapi_push_strtable too big");
        }

        T->entries = mapi_allocator_vec(
            mapi_instance(U),
            T->entries,
            T->size + EXPANSION_FACTOR,
            sizeof(struct mc_strtable_entry)
        );

        T->size += EXPANSION_FACTOR;
    }

    char *buffer = mapi_allocator_vec(mapi_instance(U), NULL, size + 1, sizeof(char));
    struct entry entry = {
        .string = buffer,
        .size = size
    };

    memcpy(buffer, str, size * sizeof(char));
    buffer[size] = '\0';

    T->entries[T->used] = entry;
    T->used++;

    return T->used - 1;
}

MORPHINE_API struct mc_strtable_entry mcapi_strtable_access(
    morphine_coroutine_t U,
    struct mc_strtable *T,
    mc_strtable_index_t index
) {
    if (index >= T->used) {
        mapi_error(U, "string not found");
    }

    struct entry entry = T->entries[index];
    return (struct mc_strtable_entry) {
        .string = entry.string,
        .size = entry.size
    };
}
