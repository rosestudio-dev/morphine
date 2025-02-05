//
// Created by why-iskra on 05.01.2025.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"
#include "morphine/utils/ctype.h"

#define BASE16_SCHEME ("0123456789abcdef")
#define BASE32_SCHEME ("abcdefghijklmnopqrstuvwxyz234567")
#define BASE64_SCHEME ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
#define BASE_PADDING  ("=")

static inline char byte2scheme(char value, char const *scheme, size_t size) {
    return scheme[(size_t) value % size];
}

static inline int scheme2byte(char value, char const *scheme, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (value == scheme[i]) {
            return (char) i;
        }
    }

    return -1;
}

static void lib_base16_encode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size size = mapi_string_len(U);
            const char *string = mapi_get_string(U);

            mapi_push_string(U, "");
            for (ml_size i = 0; i < size; i++) {
                char byte[2] = {
                    byte2scheme((char) (string[i] >> 4 & 0xf), BASE16_SCHEME, 16),
                    byte2scheme((char) (string[i] >> 0 & 0xf), BASE16_SCHEME, 16),
                };

                mapi_push_stringn(U, byte, 2);
                mapi_string_concat(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void lib_base16_decode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size size = mapi_string_len(U);
            const char *string = mapi_get_string(U);

            if (size % 2 != 0) {
                goto error;
            }

            mapi_push_string(U, "");
            for (ml_size i = 0; i < size / 2; i++) {
                int comp1 = scheme2byte(mm_ctype_tolower(string[i * 2]), BASE16_SCHEME, 16);
                int comp2 = scheme2byte(mm_ctype_tolower(string[i * 2 + 1]), BASE16_SCHEME, 16);

                if (comp1 < 0 || comp2 < 0) {
                    goto error;
                }

                char byte = (char) (((comp1 << 4) & 0xf0) | (comp2 & 0xf));

                mapi_push_stringn(U, &byte, 1);
                mapi_string_concat(U);
            }

            maux_nb_return();
    maux_nb_end

error:
    mapi_error(U, "cannot decode to base16");
}

static void lib_base32_encode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size size = mapi_string_len(U);
            const char *string = mapi_get_string(U);

            int bits = 0;
            uint32_t buffer = 0;
            mapi_push_string(U, "");
            for (ml_size i = 0; i < size; i++) {
                buffer = (buffer << 8) | (((uint32_t) string[i]) & 0xff);
                bits += 8;

                while (bits >= 5) {
                    char value = (char) ((buffer >> (bits - 5)) & 0x1f);
                    char byte = byte2scheme(value, BASE32_SCHEME, 32);

                    mapi_push_stringn(U, &byte, 1);
                    mapi_string_concat(U);

                    bits -= 5;
                }
            }

            if (bits > 0) {
                char value = (char) ((buffer << (5 - bits)) & 0x1f);
                char byte = byte2scheme(value, BASE32_SCHEME, 32);

                mapi_push_stringn(U, &byte, 1);
                mapi_string_concat(U);
            }

            while (mapi_string_len(U) % 8 != 0) {
                mapi_push_string(U, BASE_PADDING);
                mapi_string_concat(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void lib_base32_decode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size size = mapi_string_len(U);
            const char *string = mapi_get_string(U);

            int bits = 0;
            uint32_t buffer = 0;
            bool padding = false;
            mapi_push_string(U, "");
            for (ml_size i = 0; i < size; i++) {
                char value = string[i];

                if (value == '=') {
                    padding = true;
                    continue;
                } else if (padding) {
                    goto error;
                }

                {
                    int byte = scheme2byte(mm_ctype_tolower(value), BASE32_SCHEME, 32);
                    if (byte < 0) {
                        goto error;
                    }

                    buffer = buffer << 5 | (((uint32_t) byte) & 0xff);
                    bits += 5;
                }

                if (bits >= 8) {
                    char byte = (char) ((buffer >> (bits - 8)) & 0xff);

                    mapi_push_stringn(U, &byte, 1);
                    mapi_string_concat(U);

                    bits -= 8;
                }
            }

            maux_nb_return();
    maux_nb_end

error:
    mapi_error(U, "cannot decode to base32");
}

static void lib_base64_encode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size size = mapi_string_len(U);
            const char *string = mapi_get_string(U);

            int bits = 0;
            uint32_t buffer = 0;
            mapi_push_string(U, "");
            for (ml_size i = 0; i < size; i++) {
                buffer = (buffer << 8) | (((uint32_t) string[i]) & 0xff);
                bits += 8;

                while (bits >= 6) {
                    char value = (char) ((buffer >> (bits - 6)) & 0x3f);
                    char byte = byte2scheme(value, BASE64_SCHEME, 64);

                    mapi_push_stringn(U, &byte, 1);
                    mapi_string_concat(U);

                    bits -= 6;
                }
            }

            if (bits > 0) {
                char value = (char) ((buffer << (6 - bits)) & 0x3f);
                char byte = byte2scheme(value, BASE64_SCHEME, 64);

                mapi_push_stringn(U, &byte, 1);
                mapi_string_concat(U);
            }

            while (mapi_string_len(U) % 4 != 0) {
                mapi_push_string(U, BASE_PADDING);
                mapi_string_concat(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void lib_base64_decode(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size size = mapi_string_len(U);
            const char *string = mapi_get_string(U);

            int bits = 0;
            uint32_t buffer = 0;
            bool padding = false;
            mapi_push_string(U, "");
            for (ml_size i = 0; i < size; i++) {
                char value = string[i];

                if (value == '=') {
                    padding = true;
                    continue;
                } else if (padding) {
                    goto error;
                }

                {
                    int byte = scheme2byte(value, BASE64_SCHEME, 64);
                    if (byte < 0) {
                        goto error;
                    }

                    buffer = buffer << 6 | (((uint32_t) byte) & 0xff);
                    bits += 6;
                }

                if (bits >= 8) {
                    char byte = (char) ((buffer >> (bits - 8)) & 0xff);

                    mapi_push_stringn(U, &byte, 1);
                    mapi_string_concat(U);

                    bits -= 8;
                }
            }

            maux_nb_return();
    maux_nb_end

error:
    mapi_error(U, "cannot decode to base64");
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("base16.encode", lib_base16_encode),
    MAUX_CONSTRUCT_FUNCTION("base16.decode", lib_base16_decode),

    MAUX_CONSTRUCT_FUNCTION("base32.encode", lib_base32_encode),
    MAUX_CONSTRUCT_FUNCTION("base32.decode", lib_base32_decode),

    MAUX_CONSTRUCT_FUNCTION("base64.encode", lib_base64_encode),
    MAUX_CONSTRUCT_FUNCTION("base64.decode", lib_base64_decode),

    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_algorithm(void) {
    return (morphine_library_t) {
        .name = "algorithm",
        .sharedkey = NULL,
        .init = library_init
    };
}
