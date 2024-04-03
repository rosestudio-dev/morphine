//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/userdata.h"
#include "morphine/core/allocator.h"
#include "morphine/core/throw.h"
#include "morphine/gc/barrier.h"

struct userdata *userdataI_create(
    morphine_instance_t I,
    const char *type,
    void *p,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
) {
    if (type == NULL) {
        throwI_message_panic(I, NULL, "Userdata type is null");
    }

    size_t type_len = strlen(type) + 1;

    struct userdata *result = allocI_uni(I, NULL, sizeof(struct userdata) + type_len);

    (*result) = (struct userdata) {
        .type = ((void *) result) + sizeof(struct userdata),
        .data = p,
        .free = free,
        .mark = mark,
        .links.size = 0,
        .links.pool = NULL,
        .metatable = NULL,
    };

    memset(result->type, '\0', type_len);
    strcpy(result->type, type);

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
        throwI_message_panic(I, NULL, "Userdata is null");
    }

    if (linking == NULL) {
        throwI_message_panic(I, NULL, "Linking userdata is null");
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

    userdata->links.pool = link;
    userdata->links.size++;
}

bool userdataI_unlink(morphine_instance_t I, struct userdata *userdata, void *pointer) {
    if (userdata == NULL) {
        throwI_message_panic(I, NULL, "Userdata is null");
    }

    struct link *current = userdata->links.pool;
    struct link *pool = NULL;
    bool found = false;
    while (current != NULL) {
        struct link *prev = current->prev;

        if (current->userdata->data == pointer) {
            userdata->links.size--;
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
