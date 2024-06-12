//
// Created by why-iskra on 12.06.2024.
//

#include "morphinec/rollout.h"

MORPHINE_API ml_size mcapi_rollout(morphine_coroutine_t U) {
    mapi_push_table(U);
    mapi_table_mode_fixed(U, false);
    mapi_rotate(U, 2);

    mapi_extract_callable(U);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);

    mapi_push_index(U, 0);

    mapi_table_set(U);

    ml_size count = 1;
    for (ml_size i = 0; i < mapi_table_len(U); i++) {
        mapi_table_idx_keyoe(U, i);

        ml_size constants = mapi_constant_size(U);
        for (ml_size c = 0; c < constants; c++) {
            mapi_constant_get(U, c);

            if (!mapi_is_type(U, "function")) {
                mapi_pop(U, 1);
                continue;
            }

            mapi_peek(U, 2);
            mapi_rotate(U, 2);

            bool has = mapi_table_get(U);
            mapi_pop(U, 1);
            if (!has) {
                mapi_peek(U, 1);
                mapi_constant_get(U, c);

                mapi_rotate(U, 2);
                mapi_pop(U, 1);

                mapi_push_index(U, count);

                count++;
                mapi_table_set(U);
                mapi_pop(U, 1);
            } else {
                mapi_pop(U, 1);
            }
        }

        mapi_pop(U, 1);
    }

    return count;
}

MORPHINE_API ml_size mcapi_rollout_as_vector(morphine_coroutine_t U) {
    ml_size count = mcapi_rollout(U);

    ml_size table_size = mapi_table_len(U);
    mapi_push_vector(U, count);

    for (ml_size i = 0; i < table_size; i++) {
    mapi_peek(U, 1);
    mapi_table_idx_keyoe(U, i);
    mapi_rotate(U, 2);
    mapi_pop(U, 1);

    mapi_vector_set(U, i);
    }

    mapi_rotate(U, 2);
    mapi_pop(U, 1);

    return count;
}
