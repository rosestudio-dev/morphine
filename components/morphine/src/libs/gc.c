//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/builtin.h"

static void control_full(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_gc_full(mapi_instance(U));
            maux_nb_leave();
    maux_nb_end
}

static void control_force(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_gc_force(mapi_instance(U));
            maux_nb_leave();
    maux_nb_end
}

static void control_step(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            size_t count = 1;
            if (mapi_args(U) > 0) {
                maux_expect_args(U, 1);

                mapi_push_arg(U, 0);
                count = mapi_get_size(U, "count");
            }

            for (size_t i = 0; i < count; i++) {
                mapi_gc_work(mapi_instance(U));
            }

            maux_nb_leave();
    maux_nb_end
}

static void control_enable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool value = true;
            if (mapi_args(U) > 0) {
                maux_expect_args(U, 1);

                mapi_push_arg(U, 0);
                maux_expect(U, MTYPE_BOOLEAN);

                value = mapi_get_boolean(U);
            }

            if (value) {
                mapi_gc_enable(mapi_instance(U));
            } else {
                mapi_gc_disable(mapi_instance(U));
            }
            maux_nb_leave();
    maux_nb_end
}

static void status_get(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            const char *result = mapi_gc_status(mapi_instance(U));
            maux_nb_return(mapi_push_string(U, result));
    maux_nb_end
}

static void status_isrunning(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            bool result = mapi_gc_is_running(mapi_instance(U));
            maux_nb_return(mapi_push_boolean(U, result));
    maux_nb_end
}

static void status_isenabled(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            bool result = mapi_gc_is_enabled(mapi_instance(U));
            maux_nb_return(mapi_push_boolean(U, result));
    maux_nb_end
}

static void settings_setlimit(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t limit = mapi_get_csize(U, NULL);
            mapi_gc_full(mapi_instance(U));
            mapi_gc_set_limit(mapi_instance(U), limit);
            maux_nb_leave();
    maux_nb_end
}

static void settings_setthreshold(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t value = mapi_get_csize(U, NULL);
            mapi_gc_set_threshold(mapi_instance(U), value);
            maux_nb_leave();
    maux_nb_end
}

static void settings_setdeal(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t value = mapi_get_csize(U, NULL);
            mapi_gc_set_deal(mapi_instance(U), value > UINT16_MAX ? UINT16_MAX : (uint16_t) value);
            maux_nb_leave();
    maux_nb_end
}

static void settings_setgrow(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t value = mapi_get_csize(U, NULL);
            mapi_gc_set_grow(mapi_instance(U), value > UINT16_MAX ? UINT16_MAX : (uint16_t) value);
            maux_nb_leave();
    maux_nb_end
}

static void settings_setpause(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            size_t value = mapi_get_csize(U, NULL);
            mapi_gc_set_pause(mapi_instance(U), value > UINT8_MAX ? UINT8_MAX : (uint8_t) value);
            maux_nb_leave();
    maux_nb_end
}

static void stat_memory_current(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            size_t result = mapi_gc_allocated(mapi_instance(U));
            maux_nb_return(mapi_push_csize(U, result, NULL));
    maux_nb_end
}

static void stat_memory_peak(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            size_t result = mapi_gc_max_allocated(mapi_instance(U));
            maux_nb_return(mapi_push_csize(U, result, NULL));
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("control.full", control_full),
    MAUX_CONSTRUCT_FUNCTION("control.force", control_force),
    MAUX_CONSTRUCT_FUNCTION("control.step", control_step),
    MAUX_CONSTRUCT_FUNCTION("control.enable", control_enable),
    MAUX_CONSTRUCT_FUNCTION("status.get", status_get),
    MAUX_CONSTRUCT_FUNCTION("status.isrunning", status_isrunning),
    MAUX_CONSTRUCT_FUNCTION("status.isenabled", status_isenabled),
    MAUX_CONSTRUCT_FUNCTION("settings.setlimit", settings_setlimit),
    MAUX_CONSTRUCT_FUNCTION("settings.setthreshold", settings_setthreshold),
    MAUX_CONSTRUCT_FUNCTION("settings.setdeal", settings_setdeal),
    MAUX_CONSTRUCT_FUNCTION("settings.setgrow", settings_setgrow),
    MAUX_CONSTRUCT_FUNCTION("settings.setpause", settings_setpause),
    MAUX_CONSTRUCT_FUNCTION("stat.memory.current", stat_memory_current),
    MAUX_CONSTRUCT_FUNCTION("stat.memory.peak", stat_memory_peak),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_gc(void) {
    return (morphine_library_t) {
        .name = "gc",
        .sharedkey = NULL,
        .init = library_init
    };
}
