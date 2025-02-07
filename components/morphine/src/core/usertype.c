//
// Created by why-iskra on 21.07.2024.
//

#include <string.h>
#include "morphine/core/usertype.h"
#include "morphine/core/instance.h"
#include "morphine/gc/allocator.h"

struct usertypes usertypeI_prototype(void) {
    return (struct usertypes) {
        .list = NULL
    };
}

static void usertype_free(morphine_instance_t I, struct usertype *usertype) {
    allocI_free(I, usertype);
}

void usertypeI_free(morphine_instance_t I, struct usertypes *usertypes) {
    struct usertype *usertype = usertypes->list;
    while (usertype != NULL) {
        struct usertype *prev = usertype->prev;
        usertype_free(I, usertype);

        usertype = prev;
    }
}

void usertypeI_declare(
    morphine_instance_t I,
    const char *name,
    size_t allocate,
    mfunc_constructor_t constructor,
    mfunc_destructor_t destructor,
    struct table *metatable
) {
    if (valueI_is_type(I, name, true)) {
        throwI_error(I, "unavailable type name");
    }

    {
        struct usertype *usertype = I->usertypes.list;
        while (usertype != NULL) {
            if (strcmp(usertype->name, name) == 0) {
                bool check = usertype->allocate == allocate &&
                             usertype->constructor == constructor &&
                             usertype->destructor == destructor &&
                             usertype->metatable == metatable;

                if (check) {
                    return;
                } else {
                    throwI_error(I, "type is already declared");
                }
            }

            usertype = usertype->prev;
        }
    }

    size_t name_len = strlen(name);

    if (name_len > MPARAM_USERTYPE_NAME_LIMIT) {
        throwI_error(I, "name of type too big");
    }

    size_t size = sizeof(struct usertype) + sizeof(char) * (name_len + 1);
    struct usertype *usertype = allocI_uni(I, NULL, size);

    char *name_str = ((void *) usertype) + sizeof(struct usertype);
    (*usertype) = (struct usertype) {
        .name = name_str,
        .constructor = constructor,
        .destructor = destructor,
        .allocate = allocate,
        .metatable = metatable,
        .prev = I->usertypes.list
    };

    memcpy(name_str, name, sizeof(char) * name_len);
    name_str[name_len] = '\0';

    I->usertypes.list = usertype;
}

bool usertypeI_has(morphine_instance_t I, const char *name) {
    struct usertype *usertype = I->usertypes.list;
    while (usertype != NULL) {
        if (strcmp(usertype->name, name) == 0) {
            return true;
        }

        usertype = usertype->prev;
    }

    return false;
}

struct usertype *usertypeI_get(morphine_instance_t I, const char *name) {
    struct usertype *usertype = I->usertypes.list;
    while (usertype != NULL) {
        if (strcmp(usertype->name, name) == 0) {
            return usertype;
        }

        usertype = usertype->prev;
    }

    throwI_error(I, "type isn't declared");
}
