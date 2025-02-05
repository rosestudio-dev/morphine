//
// Created by whyiskra on 30.12.23.
//

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "morphinel/termios.h"

static struct termios get_termios(morphine_coroutine_t U) {
    struct termios termios;
    if (tcgetattr(STDIN_FILENO, &termios)) {
        mapi_error(U, "cannot get terminal parameters");
    }

    return termios;
}

static void set_termios(morphine_coroutine_t U, struct termios termios) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios)) {
        mapi_error(U, "cannot set terminal parameters");
    }
}

static void lib_makeraw(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 0);

            struct termios termios = get_termios(U);
            termios.c_iflag &= (tcflag_t) ~(
                IGNBRK | BRKINT | IGNPAR | PARMRK |
                INPCK | ISTRIP | INLCR | IGNCR |
                ICRNL | IXON | IXANY | IXOFF
            );
            termios.c_oflag &= (tcflag_t) ~OPOST;
            termios.c_cflag &= (tcflag_t) ~(PARENB | CSIZE);
            termios.c_cflag |= CS8;
            termios.c_lflag &= (tcflag_t) ~(
                ECHO | ECHOE | ECHOK | ECHONL |
                ICANON | IEXTEN | ISIG | NOFLSH |
                TOSTOP
            );
            termios.c_cc[VMIN] = 1;
            termios.c_cc[VTIME] = 0;
            set_termios(U, termios);

            maux_nb_leave();
    maux_nb_end
}

static void lib_makecbreak(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 0);

            struct termios termios = get_termios(U);
            termios.c_lflag &= (tcflag_t) ~(ECHO | ICANON);
            termios.c_cc[VMIN] = 1;
            termios.c_cc[VTIME] = 0;
            set_termios(U, termios);

            maux_nb_leave();
    maux_nb_end
}

static void lib_size(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 0);

            struct winsize winsize;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize)) {
                mapi_push_nil(U);
            } else {
                mapi_push_table(U);

                mapi_push_integer(U, winsize.ws_row);
                maux_table_set(U, "row");

                mapi_push_integer(U, winsize.ws_col);
                maux_table_set(U, "col");
            }

            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("makeraw", lib_makeraw),
    MAUX_CONSTRUCT_FUNCTION("makecbreak", lib_makecbreak),
    MAUX_CONSTRUCT_FUNCTION("size", lib_size),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mllib_termios(void) {
    return (morphine_library_t) {
        .name = "termios",
        .sharedkey = NULL,
        .init = library_init
    };
}
