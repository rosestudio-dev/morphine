//
// Created by whyiskra on 17.12.23.
//

#include "morphine/core/call.h"
#include "morphine/object/table.h"
#include "morphine/core/metatable.h"
#include "morphine/core/instance.h"

void callI_do(
    morphine_state_t S,
    struct value callable,
    struct value self,
    size_t argc,
    struct value *args,
    size_t pop_size
) {
    struct value mt_field;
    if (morphinem_unlikely(metatableI_test(S->I, callable, MF_CALL, &mt_field))) {
        struct value table = valueI_object(tableI_create(S->I, argc));
        stackI_push(S, table);

        for (size_t i = 0; i < argc; i++) {
            tableI_set(S->I, valueI_as_table_or_error(S, table), valueI_size2integer(S, i), args[i]);
        }

        struct value new_args[2] = { self, table };

        stackI_call(S, mt_field, callable, 2, new_args, pop_size + 1);
    } else {
        stackI_call(S, callable, self, argc, args, pop_size);
    }
}
