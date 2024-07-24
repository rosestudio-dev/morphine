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
    morphine_free_t free,
    bool require_metatable
) {
    if (valueI_is_type(I, name, true)) {
        throwI_error(I, "unavailable type name");
    }

    size_t name_len = strlen(name);

    {
        struct usertype *usertype = I->usertypes.types;
        while (usertype != NULL) {
            if (name_len == usertype->info.name_len && memcmp(usertype->info.name, name, name_len) == 0) {
                throwI_error(I, "type is already declared");
            }

            usertype = usertype->prev;
        }
    }

    if (name_len > MLIMIT_USERTYPE_NAME) {
        throwI_error(I, "name of type too big");
    }

    size_t size = sizeof(struct usertype) + sizeof(char) * (name_len + 1);
    struct usertype *usertype = allocI_uni(I, NULL, size);

    char *name_str = ((void *) usertype) + sizeof(struct usertype);
    (*usertype) = (struct usertype) {
        .info.name = name_str,
        .info.name_len = name_len,
        .info.free = free,
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
    size_t name_len = strlen(name);
    struct usertype *usertype = I->usertypes.types;
    while (usertype != NULL) {
        if (name_len == usertype->info.name_len && memcmp(usertype->info.name, name, name_len) == 0) {
            return true;
        }

        usertype = usertype->prev;
    }

    return false;
}

struct usertype *usertypeI_get(morphine_instance_t I, const char *name) {
    size_t name_len = strlen(name);
    struct usertype *usertype = I->usertypes.types;
    while (usertype != NULL) {
        if (name_len == usertype->info.name_len && memcmp(usertype->info.name, name, name_len) == 0) {
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
