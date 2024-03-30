//
// Created by whyiskra on 25.12.23.
//

#include <string.h>
#include "morphine/core/require.h"
#include "morphine/core/libloaders.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/state.h"
#include "morphine/stack/call.h"
#include "morphine/stack/access.h"

static struct require_loader table[] = {
    { "base",      mlib_base_loader },
    { "gc",        mlib_gc_loader },
    { "coroutine", mlib_coroutine_loader },
    { "math",      mlib_math_loader },
    { "string",    mlib_string_loader },
    { "table",     mlib_table_loader },
    { "value",     mlib_value_loader },
    { "registry",  mlib_registry_loader },
    { NULL, NULL }
};

static inline struct require_loader *search(struct require_loader *loader, const char *str_id) {
    struct require_loader *current = loader;
    while (current->name != NULL) {
        if (strcmp(str_id, current->name) == 0) {
            return current;
        }

        current++;
    }

    return NULL;
}

struct value requireI_load(morphine_state_t S, struct value id) {
    bool has;

    // check cache

    stackI_push(S, id);

    struct value key = valueI_object(stringI_create(S->I, "_LOADED"));
    struct value env = *callstackI_info_or_error(S)->s.env.p;

    stackI_push(S, key);

    struct value cache_section = tableI_get(S->I, valueI_as_table_or_error(S, env), key, &has);

    if (!has) {
        cache_section = valueI_object(tableI_create(S->I));
        stackI_push(S, cache_section);
        tableI_set(S->I, valueI_as_table_or_error(S, env), key, cache_section);
        stackI_pop(S, 1);
    }

    stackI_pop(S, 1);

    stackI_push(S, cache_section);

    // load

    struct value module = tableI_get(S->I, valueI_as_table_or_error(S, cache_section), id, &has);

    if (has) {
        stackI_pop(S, 2);
        return module;
    }

    const char *str_id = valueI_as_string_or_error(S, id)->chars;
    struct require_loader *found = search(table, str_id);

    struct require_loader *usertable = S->I->require_loader_table;
    if (found == NULL && usertable != NULL) {
        found = search(usertable, str_id);
    }

    if (found != NULL) {
        found->loader(S);
        module = stackI_peek(S, 0);

        tableI_set(S->I, valueI_as_table_or_error(S, cache_section), id, module);

        stackI_pop(S, 3);
        return module;
    } else {
        throwI_errorf(S, "Require didn't find module with '%s' id", str_id);
    }
}
