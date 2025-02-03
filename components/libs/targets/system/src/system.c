//
// Created by whyiskra on 30.12.23.
//

#include "morphinel/system.h"
#include <sys/time.h>
#include <sys/utsname.h>

static uint64_t get_millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((uint64_t) tv.tv_sec) * 1000) + (((uint64_t) tv.tv_usec) / 1000);
}

static void lib_delay(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            uint64_t *time = mapi_push_userdata_uni(U, sizeof(uint64_t));
            *time = get_millis();
            maux_nb_im_continue(1);
        maux_nb_state(1)
            uint64_t *time = mapi_userdata_pointer(U, NULL);

            mapi_push_arg(U, 0);
            ml_size size = mapi_get_size(U, "millis");
            mapi_pop(U, 1);

            if (get_millis() - *time < size) {
                maux_nb_continue(1);
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_millis(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_integer(U, (ml_integer) (get_millis() % (uint64_t) mm_typemax(ml_integer)));
            maux_nb_return();
    maux_nb_end
}

static void lib_uname(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);

            struct utsname uts;
            if (uname(&uts)) {
                mapi_error(U, "cannot get information");
            }

            maux_construct_element_t elements[] = {
                MAUX_CONSTRUCT_STRING("machine", uts.machine),
                MAUX_CONSTRUCT_STRING("nodename", uts.nodename),
                MAUX_CONSTRUCT_STRING("release", uts.release),
                MAUX_CONSTRUCT_STRING("sysname", uts.sysname),
                MAUX_CONSTRUCT_STRING("version", uts.version),
                MAUX_CONSTRUCT_END
            };

            maux_construct(U, elements);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("millis", lib_millis),
    MAUX_CONSTRUCT_FUNCTION("delay", lib_delay),
    MAUX_CONSTRUCT_FUNCTION("uname", lib_uname),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mllib_system(void) {
    return (morphine_library_t) {
        .name = "system",
        .sharedkey = NULL,
        .init = library_init
    };
}
