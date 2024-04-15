//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/userdata.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"

struct userdata *userdataI_create(
    morphine_instance_t I,
    const char *name,
    void *p,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
) {
    if (name == NULL) {
        throwI_error(I, "Userdata name is null");
    }

    size_t name_len = strlen(name);

    if (name_len > MLIMIT_USERDATA_NAME) {
        throwI_error(I, "Native name too big");
    }

    size_t alloc_size = sizeof(struct userdata) + ((name_len + 1) * sizeof(char));
    struct userdata *result = allocI_uni(I, NULL, alloc_size);

    (*result) = (struct userdata) {
        .name = ((void *) result) + sizeof(struct userdata),
        .name_len = name_len,
        .data = p,
        .free = free,
        .mark = mark,
        .links.size = 0,
        .links.pool = NULL,
        .metatable = NULL,
    };

    memset(result->name, 0, (name_len + 1) * sizeof(char));
    strcpy(result->name, name);

    objectI_init(I, objectI_cast(result), OBJ_TYPE_USERDATA);

    return result;
}

void userdataI_free(morphine_instance_t I, struct userdata *userdata) {
    if (userdata->free != NULL) {
        userdata->free(I, userdata->data);
    }

    struct link *current = userdata->links.pool;
    while (current != NULL) {
        struct link *prev = current->prev;

        allocI_free(I, current);

        current = prev;
    }

    allocI_free(I, userdata);
}

void userdataI_link(morphine_instance_t I, struct userdata *userdata, struct userdata *linking, bool soft) {
    if (userdata == NULL) {
        throwI_error(I, "Userdata is null");
    }

    if (linking == NULL) {
        throwI_error(I, "Linking userdata is null");
    }

    gcI_objbarrier(userdata, linking);

    struct link *current = userdata->links.pool;
    while (current != NULL) {
        if (current->userdata == linking) {
            return;
        }

        current = current->prev;
    }

    struct link *link = allocI_uni(I, NULL, sizeof(struct link));

    (*link) = (struct link) {
        .userdata = linking,
        .soft = soft,
        .prev = userdata->links.pool
    };

    userdata->links.size ++;
    userdata->links.pool = link;
}

bool userdataI_unlink(morphine_instance_t I, struct userdata *userdata, void *pointer) {
    if (userdata == NULL) {
        throwI_error(I, "Userdata is null");
    }

    struct link *current = userdata->links.pool;
    struct link *pool = NULL;
    bool found = false;
    while (current != NULL) {
        struct link *prev = current->prev;

        if (current->userdata->data == pointer) {
            userdata->links.size --;
            allocI_free(I, current);
            found = true;
        } else {
            current->prev = pool;
            pool = current;
        }

        current = prev;
    }

    userdata->links.pool = pool;

    return found;
}
