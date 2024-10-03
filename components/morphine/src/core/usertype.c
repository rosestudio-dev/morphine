//
// Created by why-iskra on 21.07.2024.
//

#include <string.h>
#include "morphine/core/usertype.h"
#include "morphine/core/instance.h"
#include "morphine/gc/allocator.h"

struct usertype {
    struct usertype_info info;
    size_t references;

    struct usertype *prev;
};

static void usertype_free(morphine_instance_t I, struct usertype *usertype) {
    allocI_free(I, usertype);
}

struct usertypes usertypeI_prototype(void) {
    return (struct usertypes) {
        .types = NULL
    };
}

void usertypeI_free(morphine_instance_t I, struct usertypes *usertypes) {
    struct usertype *usertype = usertypes->types;
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
    morphine_userdata_init_t init,
    morphine_userdata_free_t free,
    morphine_userdata_compare_t compare,
    morphine_userdata_hash_t hash,
    bool require_metatable
) {
    if (valueI_is_type(I, name, true)) {
        throwI_error(I, "unavailable type name");
    }

    {
        struct usertype *usertype = I->usertypes.types;
        while (usertype != NULL) {
            if (strcmp(usertype->info.name, name) == 0) {
                bool check = usertype->info.allocate == allocate &&
                             usertype->info.init == init &&
                             usertype->info.free == free &&
                             usertype->info.compare == compare &&
                             usertype->info.hash == hash &&
                             usertype->info.require_metatable == require_metatable;

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

    if (name_len > MLIMIT_USERTYPE_NAME) {
        throwI_error(I, "name of type too big");
    }

    size_t size = sizeof(struct usertype) + sizeof(char) * (name_len + 1);
    struct usertype *usertype = allocI_uni(I, NULL, size);

    char *name_str = ((void *) usertype) + sizeof(struct usertype);
    (*usertype) = (struct usertype) {
        .info.name = name_str,
        .info.init = init,
        .info.free = free,
        .info.compare = compare,
        .info.hash = hash,
        .info.allocate = allocate,
        .info.require_metatable = require_metatable,

        .references = 0,

        .prev = I->usertypes.types
    };

    memcpy(name_str, name, sizeof(char) * name_len);
    name_str[name_len] = '\0';

    I->usertypes.types = usertype;
}

bool usertypeI_is_declared(morphine_instance_t I, const char *name) {
    struct usertype *usertype = I->usertypes.types;
    while (usertype != NULL) {
        if (strcmp(usertype->info.name, name) == 0) {
            return true;
        }

        usertype = usertype->prev;
    }

    return false;
}

struct usertype *usertypeI_get(morphine_instance_t I, const char *name) {
    struct usertype *usertype = I->usertypes.types;
    while (usertype != NULL) {
        if (strcmp(usertype->info.name, name) == 0) {
            return usertype;
        }

        usertype = usertype->prev;
    }

    throwI_error(I, "type isn't declared");
}

struct usertype_info usertypeI_info(morphine_instance_t I, struct usertype *usertype) {
    if (usertype == NULL) {
        throwI_panic(I, "type is null");
    }

    return usertype->info;
}

void usertypeI_ref(morphine_instance_t I, struct usertype *usertype) {
    if (usertype == NULL) {
        throwI_panic(I, "type is null");
    }

    usertype->references++;
}

void usertypeI_unref(morphine_instance_t I, struct usertype *usertype) {
    if (usertype == NULL) {
        throwI_panic(I, "type is null");
    }

    if (usertype->references == 0) {
        throwI_panic(I, "references of type was corrupted");
    }

    usertype->references--;
}

bool usertypeI_eq(struct usertype *a, struct usertype *b) {
    if (a == NULL || b == NULL) {
        return false;
    }

    return a == b;
}
