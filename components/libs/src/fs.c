//
// Created by why-iskra on 31.07.2024.
//

#include <morphinel/fs.h>
#include "morphinel/fs/file.h"

static void file(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            const char *mode = mapi_get_string(U);
            ml_size mode_size = mapi_string_len(U);

            bool read = false;
            bool write = false;
            bool binary = false;
            for (ml_size i = 0; i < mode_size; i++) {
                switch (mode[i]) {
                    case 'r':
                        read = true;
                        break;
                    case 'w':
                        write = true;
                        break;
                    case 'b':
                        binary = true;
                        break;
                    default:
                        mapi_error(U, "unsupported file mode");
                }
            }

            mapi_push_arg(U, 0);
            mlapi_fs_file(U, read, write, binary);
            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "file", file },
};

static morphine_library_t library = {
    .name = "fs",
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mllib_fs(void) {
    return &library;
}
