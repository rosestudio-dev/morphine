//
// Created by why-iskra on 31.07.2024.
//

#include "morphinel/fs.h"
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <errno.h>
#include <stdio.h>

#define FS_SEP           "/"
#define DIR_DEFAULT_PERM (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH)

#define SAFEDIR_TYPE "fs-internal-safedir"

struct safedir {
    DIR *dir;
};

static void permission2string(mode_t mode, char *result) {
    for (int i = 0; i < 9; i++) {
        result[i] = '-';
    }
    result[9] = '\0';

    if (mode & S_IRUSR) {
        result[0] = 'r';
    }

    if (mode & S_IWUSR) {
        result[1] = 'w';
    }

    if (mode & S_IXUSR) {
        result[2] = 'x';
    }

    if (mode & S_IRGRP) {
        result[3] = 'r';
    }

    if (mode & S_IWGRP) {
        result[4] = 'w';
    }

    if (mode & S_IXGRP) {
        result[5] = 'x';
    }

    if (mode & S_IROTH) {
        result[6] = 'r';
    }

    if (mode & S_IWOTH) {
        result[7] = 'w';
    }

    if (mode & S_IXOTH) {
        result[8] = 'x';
    }
}

static const char *type2string(mode_t mode) {
    const char *type = "other";
    if (S_ISDIR(mode)) {
        type = "dir";
    } else if (S_ISREG(mode)) {
        type = "file";
    } else if (S_ISLNK(mode)) {
        type = "link";
    } else if (S_ISCHR(mode)) {
        type = "char";
    } else if (S_ISBLK(mode)) {
        type = "block";
    } else if (S_ISFIFO(mode)) {
        type = "pipe";
    }

    return type;
}

static bool isdir(morphine_coroutine_t U, const char *path) {
    struct stat st;
    if (stat(path, &st)) {
        mapi_errorf(U, "cannot get type of %s", path);
    }

    return S_ISDIR(st.st_mode);
}

static mode_t build_permissions_table(morphine_coroutine_t U) {
    mode_t result = 0;

    if (maux_table_has(U, "user")) {
        mapi_peek(U, 0);
        if (maux_table_access(U, "user.read") && mapi_get_boolean(U)) {
            result |= S_IRUSR;
        }
        mapi_pop(U, 1);

        mapi_peek(U, 0);
        if (maux_table_access(U, "user.write") && mapi_get_boolean(U)) {
            result |= S_IWUSR;
        }
        mapi_pop(U, 1);

        mapi_peek(U, 0);
        if (maux_table_access(U, "user.execute") && mapi_get_boolean(U)) {
            result |= S_IXUSR;
        }
        mapi_pop(U, 1);
    }

    if (maux_table_has(U, "group")) {
        mapi_peek(U, 0);
        if (maux_table_access(U, "group.read") && mapi_get_boolean(U)) {
            result |= S_IRGRP;
        }
        mapi_pop(U, 1);

        mapi_peek(U, 0);
        if (maux_table_access(U, "group.write") && mapi_get_boolean(U)) {
            result |= S_IWGRP;
        }
        mapi_pop(U, 1);

        mapi_peek(U, 0);
        if (maux_table_access(U, "group.execute") && mapi_get_boolean(U)) {
            result |= S_IXGRP;
        }
        mapi_pop(U, 1);
    }

    if (maux_table_has(U, "other")) {
        mapi_peek(U, 0);
        if (maux_table_access(U, "other.read") && mapi_get_boolean(U)) {
            result |= S_IROTH;
        }
        mapi_pop(U, 1);

        mapi_peek(U, 0);
        if (maux_table_access(U, "other.write") && mapi_get_boolean(U)) {
            result |= S_IWOTH;
        }
        mapi_pop(U, 1);

        mapi_peek(U, 0);
        if (maux_table_access(U, "other.execute") && mapi_get_boolean(U)) {
            result |= S_IXOTH;
        }
        mapi_pop(U, 1);
    }

    return result;
}

static mode_t build_permissions_integer(morphine_coroutine_t U) {
    mode_t result = 0;
    ml_size raw = mapi_get_size(U, "permission");

    {
        ml_size num = raw % 10;
        if (num > 7) {
            mapi_error(U, "wrong permission pattern");
        }

        if ((num & 0b001) == 0b001) {
            result |= S_IXOTH;
        }

        if ((num & 0b010) == 0b010) {
            result |= S_IWOTH;
        }

        if ((num & 0b100) == 0b100) {
            result |= S_IROTH;
        }
    }

    {
        ml_size num = (raw / 10) % 10;
        if (num > 7) {
            mapi_error(U, "wrong permission pattern");
        }

        if ((num & 0b001) == 0b001) {
            result |= S_IXGRP;
        }

        if ((num & 0b010) == 0b010) {
            result |= S_IWGRP;
        }

        if ((num & 0b100) == 0b100) {
            result |= S_IRGRP;
        }
    }

    {
        ml_size num = (raw / 100) % 10;
        if (num > 7) {
            mapi_error(U, "wrong permission pattern");
        }

        if ((num & 0b001) == 0b001) {
            result |= S_IXUSR;
        }

        if ((num & 0b010) == 0b010) {
            result |= S_IWUSR;
        }

        if ((num & 0b100) == 0b100) {
            result |= S_IRUSR;
        }
    }

    ml_size num = (raw / 1000) % 10;
    if (num != 0) {
        mapi_error(U, "wrong permission pattern");
    }

    return result;
}

static mode_t build_permissions(morphine_coroutine_t U) {
    if (mapi_is(U, "table")) {
        return build_permissions_table(U);
    }

    if (mapi_is(U, "integer")) {
        return build_permissions_integer(U);
    }

    mapi_error(U, "expected table or integer for permission");
}

static void copy_file(morphine_coroutine_t U, const char *src, const char *dest) {
    struct stat st;
    if (stat(src, &st)) {
        mapi_errorf(U, "file %s not found", src);
    }

    mapi_push_string(U, src);
    mlapi_fs_file(U, true, false, true);

    mapi_push_string(U, dest);
    mlapi_fs_file(U, false, true, true);

    uint8_t buffer[32];
    while (true) {
        mapi_rotate(U, 2);
        size_t size = mapi_stream_read(U, buffer, sizeof(buffer));
        mapi_rotate(U, 2);

        if (size == 0) {
            break;
        }

        if (mapi_stream_write(U, buffer, size) != size) {
            mapi_error(U, "copy failed");
        }
    }

    mapi_stream_close(U, false);
    mapi_stream_close(U, false);
}

static void construct_stat(morphine_coroutine_t U, struct stat st) {
    char mode[10];
    permission2string(st.st_mode, mode);
    const char *type = type2string(st.st_mode);

    maux_construct_element_t elements[13] = {
        MAUX_CONSTRUCT_STRING("type", type),
        MAUX_CONSTRUCT_INTEGER("size", st.st_size),
        MAUX_CONSTRUCT_SIZE("uid", st.st_uid),
        MAUX_CONSTRUCT_SIZE("gid", st.st_gid),
        MAUX_CONSTRUCT_STRING("permissions", mode),
        MAUX_CONSTRUCT_SIZE("dev", st.st_dev),
        MAUX_CONSTRUCT_SIZE("rdev", st.st_rdev),
        MAUX_CONSTRUCT_SIZE("ino", st.st_ino),
        MAUX_CONSTRUCT_SIZE("nlink", st.st_nlink),
        MAUX_CONSTRUCT_INTEGER("access", st.st_atime),
        MAUX_CONSTRUCT_INTEGER("modification", st.st_mtime),
        MAUX_CONSTRUCT_INTEGER("change", st.st_ctime),
        MAUX_CONSTRUCT_END
    };

    maux_construct(U, elements);
}

static void safedir_constructor(morphine_instance_t I, void *p) {
    (void) I;
    struct safedir *D = p;
    D->dir = NULL;
}

static void safedir_destructor(morphine_instance_t I, void *p) {
    (void) I;
    struct safedir *D = p;

    DIR *dir = D->dir;
    if (dir != NULL) {
        D->dir = NULL;
        closedir(dir);
    }
}

static struct safedir *create_safedir(morphine_coroutine_t U, const char *path) {
    mapi_type_declare(
        mapi_instance(U),
        SAFEDIR_TYPE,
        sizeof(struct safedir),
        false,
        safedir_constructor,
        safedir_destructor,
        NULL,
        NULL
    );

    struct safedir *D = mapi_push_userdata(U, SAFEDIR_TYPE);
    D->dir = opendir(path);

    if (D->dir == NULL) {
        mapi_errorf(U, "cannot open dir %s", path);
    }

    return D;
}

static void lib_open(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
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

static void lib_temp(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 0);
            mlapi_fs_temp(U);
            maux_nb_return();
    maux_nb_end
}

static void lib_info(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);

            struct stat st;
            if (stat(path, &st)) {
                mapi_push_nil(U);
                maux_nb_return();
            }

            construct_stat(U, st);
            maux_nb_return();
    maux_nb_end
}

static void lib_symlinkinfo(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);

            struct stat st;
            if (lstat(path, &st)) {
                mapi_push_nil(U);
                maux_nb_return();
            }

            construct_stat(U, st);
            maux_nb_return();
    maux_nb_end
}

static void lib_pwd(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 0);

            size_t size = MAXPATHLEN;
            char *path = NULL;
            while (true) {
                if (path == NULL) {
                    path = mapi_push_userdata_vec(U, size, sizeof(char));
                } else {
                    path = mapi_userdata_resize_vec(U, size, sizeof(char));
                }

                if (getcwd(path, size) != NULL) {
                    size_t len = strlen(path);
                    mapi_push_stringn(U, path, len);
                    maux_nb_return();
                }

                if (errno != ERANGE) {
                    mapi_push_nil(U);
                    maux_nb_return();
                }

                size *= 2;
            }
    maux_nb_end
}

static void lib_list(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);

            struct safedir *D = create_safedir(U, path);

            mapi_push_vector(U, 0);
            mapi_vector_mode_fixed(U, false);
            struct dirent *dirent;
            while ((dirent = readdir(D->dir)) != NULL) {
                if (strcmp(".", dirent->d_name) == 0 || strcmp("..", dirent->d_name) == 0) {
                    continue;
                }

                mapi_push_string(U, dirent->d_name);
                mapi_vector_push(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void lib_remove(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);
            if (isdir(U, path)) {
                mapi_push_integer(U, 0);
                maux_localstorage_set(U, "index");
                maux_nb_im_continue(1);
            } else {
                maux_nb_im_continue(5);
            }
        maux_nb_state(1);
            maux_library_access(U, "fs.list");
            mapi_push_arg(U, 0);
            maux_nb_call(1, 2);
        maux_nb_state(2);
            mapi_push_result(U);
            maux_nb_im_continue(3);
        maux_nb_state(3);
            ml_size size = mapi_vector_len(U);
            maux_localstorage_get(U, "index");
            ml_size index = mapi_get_size(U, "index");
            mapi_pop(U, 1);

            if (index >= size) {
                maux_nb_im_continue(5);
            }

            mapi_vector_get(U, index);
            mapi_push_arg(U, 0);
            mapi_push_string(U, FS_SEP);
            mapi_string_concat(U);
            mapi_rotate(U, 2);
            mapi_string_concat(U);

            mapi_push_integer(U, index + 1);
            maux_localstorage_set(U, "index");

            const char *path = mapi_get_cstr(U);
            if (isdir(U, path)) {
                mapi_push_callable(U);
                mapi_extract_source(U);
                mapi_rotate(U, 2);
                mapi_pop(U, 1);
                mapi_peek(U, 1);
                maux_nb_call(1, 4);
            } else {
                if (unlink(path)) {
                    mapi_errorf(U, "cannot remove file %s", path);
                }

                mapi_pop(U, 1);
                maux_nb_continue(3);
            }
        maux_nb_state(4);
            mapi_pop(U, 1);
            maux_nb_im_continue(3);
        maux_nb_state(5);
            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);
            if (isdir(U, path)) {
                if (rmdir(path)) {
                    mapi_errorf(U, "cannot remove dir %s", path);
                }
            } else {
                if (unlink(path)) {
                    mapi_errorf(U, "cannot remove file %s", path);
                }
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_move(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            const char *from = mapi_get_cstr(U);

            mapi_push_arg(U, 1);
            const char *to = mapi_get_cstr(U);

            if (rename(from, to)) {
                mapi_errorf(U, "cannot move %s to %s", from, to);
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_copy(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            const char *from = mapi_get_cstr(U);

            mapi_push_arg(U, 1);
            const char *to = mapi_get_cstr(U);

            copy_file(U, from, to);
            maux_nb_leave();
    maux_nb_end
}

static void lib_mkdir(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            mode_t mode = DIR_DEFAULT_PERM;
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                mode = build_permissions(U);
            } else {
                maux_expect_args(U, 1);
            }

            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);
            mapi_push_boolean(U, mkdir(path, mode));
            maux_nb_return();
    maux_nb_end
}

static void lib_chdir(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            const char *path = mapi_get_cstr(U);
            if (chdir(path)) {
                mapi_errorf(U, "cannot change dir to %s", path);
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_chmod(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            mode_t mode = build_permissions(U);

            mapi_push_arg(U, 0);
            const char *path = mapi_get_cstr(U);

            if (chmod(path, mode)) {
                mapi_errorf(U, "cannot change mode for %s", path);
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_link(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            const char *from = mapi_get_cstr(U);

            mapi_push_arg(U, 1);
            const char *to = mapi_get_cstr(U);

            if (link(from, to)) {
                mapi_errorf(U, "cannot create link from %s to %s", from, to);
            }

            maux_nb_leave();
    maux_nb_end
}

static void lib_symlink(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            const char *from = mapi_get_cstr(U);

            mapi_push_arg(U, 1);
            const char *to = mapi_get_cstr(U);

            if (symlink(from, to)) {
                mapi_errorf(U, "cannot create symlink from %s to %s", from, to);
            }

            maux_nb_leave();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("open", lib_open),
    MAUX_CONSTRUCT_FUNCTION("temp", lib_temp),
    MAUX_CONSTRUCT_FUNCTION("info", lib_info),
    MAUX_CONSTRUCT_FUNCTION("symlinkinfo", lib_symlinkinfo),
    MAUX_CONSTRUCT_FUNCTION("pwd", lib_pwd),
    MAUX_CONSTRUCT_FUNCTION("list", lib_list),
    MAUX_CONSTRUCT_FUNCTION("remove", lib_remove),
    MAUX_CONSTRUCT_FUNCTION("move", lib_move),
    MAUX_CONSTRUCT_FUNCTION("copy", lib_copy),
    MAUX_CONSTRUCT_FUNCTION("mkdir", lib_mkdir),
    MAUX_CONSTRUCT_FUNCTION("chdir", lib_chdir),
    MAUX_CONSTRUCT_FUNCTION("chmod", lib_chmod),
    MAUX_CONSTRUCT_FUNCTION("link", lib_link),
    MAUX_CONSTRUCT_FUNCTION("symlink", lib_symlink),
    MAUX_CONSTRUCT_STRING("sep", FS_SEP),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mllib_fs(void) {
    return (morphine_library_t) {
        .name = "fs",
        .sharedkey = NULL,
        .init = library_init
    };
}
