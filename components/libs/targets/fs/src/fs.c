//
// Created by why-iskra on 31.07.2024.
//

#include "morphinel/fs.h"
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

static void open(morphine_coroutine_t U) {
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

static void list(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);

            DIR *dir = opendir(path);
            if (dir == NULL) {
                mapi_error(U, "cannot open dir");
            }

            mapi_push_vector(U, 0);
            mapi_vector_mode_fixed(U, false);
            struct dirent *dirent;
            while ((dirent = readdir(dir)) != NULL) {
                if (strcmp(".", dirent->d_name) == 0 || strcmp("..", dirent->d_name) == 0) {
                    continue;
                }

                mapi_push_string(U, dirent->d_name);
                mapi_vector_push(U);
            }

            if (closedir(dir)) {
                mapi_error(U, "cannot close dir");
            }

            maux_nb_return();
    maux_nb_end
}

static void info(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);

            struct stat st;
            if (stat(path, &st)) {
                mapi_push_nil(U);
                maux_nb_return();
            }

            mapi_push_table(U);

            mapi_push_string(U, "type");
            if (S_ISDIR(st.st_mode)) {
                mapi_push_string(U, "dir");
            } else if (S_ISREG(st.st_mode)) {
                mapi_push_string(U, "file");
            } else if (S_ISLNK(st.st_mode)) {
                mapi_push_string(U, "link");
            } else if (S_ISCHR(st.st_mode)) {
                mapi_push_string(U, "csf");
            } else if (S_ISBLK(st.st_mode)) {
                mapi_push_string(U, "blk");
            } else if (S_ISFIFO(st.st_mode)) {
                mapi_push_string(U, "fifo");
            } else {
                mapi_push_string(U, "?");
            }
            mapi_table_set(U);

            mapi_push_string(U, "size");
            mapi_push_integer(U, st.st_size);
            mapi_table_set(U);

            mapi_push_string(U, "owner");
            struct passwd *pw = getpwuid(st.st_uid);
            if (pw == NULL) {
                mapi_push_size(U, st.st_uid, "uid");
            } else {
                mapi_push_string(U, pw->pw_name);
            }
            mapi_table_set(U);

            mapi_push_string(U, "access");
            mapi_push_table(U);
            mapi_push_string(U, "read");
            mapi_push_boolean(U, access(path, R_OK) == 0);
            mapi_table_set(U);
            mapi_push_string(U, "write");
            mapi_push_boolean(U, access(path, W_OK) == 0);
            mapi_table_set(U);
            mapi_push_string(U, "execute");
            mapi_push_boolean(U, access(path, X_OK) == 0);
            mapi_table_set(U);
            mapi_table_set(U);

            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "open", open },
    { "list", list },
    { "info", info },
    { NULL, NULL },
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
