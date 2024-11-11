//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/builtin.h"
#include "morphine/utils/ctype.h"

static void substring(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_push_arg(U, 1);
            size_t start = mapi_get_size(U, "index");

            mapi_push_arg(U, 2);
            size_t end = mapi_get_size(U, "index");

            mapi_pop(U, 3);

            if (start > len || len < end || start > end) {
                mapi_errorf(
                    U,
                    "cannot sub string with len %zu from %zu to %zu",
                    len,
                    start,
                    end
                );
            }

            mapi_push_stringn(U, string + start, end - start);
            maux_nb_return();
    maux_nb_end
}

static void chars(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");

            const char *string = mapi_get_string(U);
            ml_size len = mapi_string_len(U);

            mapi_push_vector(U, len);

            for (ml_size i = 0; i < len; i++) {
                mapi_push_stringf(U, "%c", string[i]);
                mapi_vector_set(U, i);
            }

            maux_nb_return();
    maux_nb_end
}

static void codes(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");

            const char *string = mapi_get_string(U);
            ml_size len = mapi_string_len(U);

            mapi_push_vector(U, len);

            for (ml_size i = 0; i < len; i++) {
                mapi_push_size(U, (size_t) ((unsigned char) string[i]), "code");
                mapi_vector_set(U, i);
            }

            maux_nb_return();
    maux_nb_end
}

static void from(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 0) {
                maux_expect_args(U, 1);
            }

            mapi_push_arg(U, 0);

            bool is_vector;
            ml_size size;
            if (mapi_args(U) == 1 && mapi_is(U, "vector")) {
                is_vector = true;
                size = mapi_vector_len(U);
            } else {
                is_vector = false;
                size = mapi_args(U);
            }

            char *result = mapi_push_userdata_vec(U, size, sizeof(char));
            mapi_rotate(U, 2);

            for (ml_size i = 0; i < size; i++) {
                if (is_vector) {
                    mapi_vector_get(U, i);
                } else {
                    mapi_push_arg(U, i);
                }

                if (mapi_is(U, "integer")) {
                    result[i] = (char) mapi_get_integer(U);
                } else if (mapi_is(U, "string")) {
                    if (mapi_string_len(U) != 1) {
                        mapi_error(U, "expected char");
                    }

                    result[i] = mapi_get_string(U)[0];
                } else {
                    mapi_error(U, "expected integer or char");
                }

                mapi_pop(U, 1);
            }

            mapi_push_stringn(U, result, size);
            maux_nb_return();
    maux_nb_end
}

static void isempty(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            size_t len = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_boolean(U, len == 0);
            maux_nb_return();
    maux_nb_end
}

static void isblankstr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_pop(U, 1);

            bool isblank = true;

            for (size_t i = 0; i < len; i++) {
                if (!morphine_isblank(string[i])) {
                    isblank = false;
                }
            }

            mapi_push_boolean(U, isblank);
            maux_nb_return();
    maux_nb_end
}

static void isdigitstr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_pop(U, 1);

            bool isdigit = true;

            for (size_t i = 0; i < len; i++) {
                if (!morphine_isdigit(string[i])) {
                    isdigit = false;
                }
            }

            mapi_push_boolean(U, isdigit);
            maux_nb_return();
    maux_nb_end
}

static void isalphastr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_pop(U, 1);

            bool isalpha = true;

            for (size_t i = 0; i < len; i++) {
                if (!morphine_isalpha(string[i])) {
                    isalpha = false;
                }
            }

            mapi_push_boolean(U, isalpha);
            maux_nb_return();
    maux_nb_end
}

static void repeat(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");

            mapi_push_arg(U, 1);
            size_t count = mapi_get_size(U, "count");

            mapi_pop(U, 2);

            mapi_push_string(U, "");
            for (size_t i = 0; i < count; i++) {
                mapi_push_arg(U, 0);

                mapi_string_concat(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void startswith(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *prefix = mapi_get_string(U);
            size_t plen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool result = strlen >= plen && memcmp(string, prefix, plen * sizeof(char)) == 0;

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void endswith(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *suffix = mapi_get_string(U);
            size_t slen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool result =
                strlen >= slen && memcmp(string + (strlen - slen), suffix, slen * sizeof(char)) == 0;

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void tolowercase(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_pop(U, 1);

            if (len > 0) {
                char *result = mapi_push_userdata_vec(U, len, sizeof(char));

                for (size_t i = 0; i < len; i++) {
                    result[i] = morphine_tolower(string[i]);
                }

                mapi_push_stringn(U, result, len);
            } else {
                mapi_push_string(U, "");
            }

            maux_nb_return();
    maux_nb_end
}

static void touppercase(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_pop(U, 1);

            if (len > 0) {
                char *result = mapi_push_userdata_vec(U, len, sizeof(char));

                for (size_t i = 0; i < len; i++) {
                    result[i] = morphine_toupper(string[i]);
                }

                mapi_push_stringn(U, result, len);
            } else {
                mapi_push_string(U, "");
            }

            maux_nb_return();
    maux_nb_end
}

static void split(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *separator = mapi_get_string(U);
            size_t seplen = mapi_string_len(U);

            mapi_pop(U, 2);

            mapi_push_vector(U, 0);
            mapi_vector_mode_fixed(U, false);
            if (strlen < seplen) {
                maux_nb_return();
            }

            size_t start = 0;

            if (seplen == 0) {
                mapi_push_string(U, "");
                mapi_vector_push(U);
            }

            for (size_t i = 0; i < strlen; i++) {
                if (i + seplen > strlen) {
                    continue;
                }

                if (seplen == 0) {
                    mapi_push_stringn(U, string + start, 1);
                    mapi_vector_push(U);
                    start++;
                } else if (memcmp(string + i, separator, seplen * sizeof(char)) == 0) {
                    mapi_push_stringn(U, string + start, i - start);
                    mapi_vector_push(U);

                    start = i + seplen;
                    i += seplen - 1;
                }
            }

            if (strlen >= start) {
                mapi_push_stringn(U, string + start, strlen - start);
            } else {
                mapi_push_string(U, "");
            }
            mapi_vector_push(U);

            maux_nb_return();
    maux_nb_end
}

static void contains(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *find = mapi_get_string(U);
            size_t findlen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool found = false;
            if (strlen >= findlen) {
                for (size_t i = 0; i < strlen - findlen + 1; i++) {
                    if (memcmp(string + i, find, findlen * sizeof(char)) == 0) {
                        found = true;
                        break;
                    }
                }
            }

            mapi_push_boolean(U, found);
            maux_nb_return();
    maux_nb_end
}

static void indexof(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *find = mapi_get_string(U);
            size_t findlen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool found = false;
            size_t index;
            if (strlen >= findlen) {
                for (size_t i = 0; i < strlen - findlen + 1; i++) {
                    if (memcmp(string + i, find, findlen * sizeof(char)) == 0) {
                        found = true;
                        index = i;
                        break;
                    }
                }
            }

            if (found) {
                mapi_push_size(U, index, "index");
            } else {
                mapi_push_nil(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void lastindexof(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *find = mapi_get_string(U);
            size_t findlen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool found = false;
            size_t index;
            if (strlen >= findlen) {
                for (size_t i = 0; i < strlen - findlen + 1; i++) {
                    if (memcmp(string + (strlen - findlen) - i, find, findlen * sizeof(char)) == 0) {
                        found = true;
                        index = (strlen - findlen) - i;
                        break;
                    }
                }
            }

            if (found) {
                mapi_push_size(U, index, "index");
            } else {
                mapi_push_nil(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void trim(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_pop(U, 1);

            size_t start = 0;
            size_t end = strlen;

            for (size_t i = 0; i < strlen; i++) {
                char c = string[i];
                if (morphine_isblank(c)) {
                    start++;
                } else {
                    break;
                }
            }

            for (size_t i = 0; i < strlen; i++) {
                char c = string[strlen - i - 1];
                if (morphine_isblank(c)) {
                    end--;
                } else {
                    break;
                }
            }

            if (start < end) {
                mapi_push_stringn(U, string + start, end - start);
            } else {
                mapi_push_string(U, "");
            }

            maux_nb_return();
    maux_nb_end
}

static void trimstart(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_pop(U, 1);

            size_t start = 0;

            for (size_t i = 0; i < strlen; i++) {
                char c = string[i];
                if (morphine_isblank(c)) {
                    start++;
                } else {
                    break;
                }
            }

            if (start < strlen) {
                mapi_push_stringn(U, string + start, strlen - start);
            } else {
                mapi_push_string(U, "");
            }

            maux_nb_return();
    maux_nb_end
}

static void trimend(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_pop(U, 1);

            size_t end = strlen;

            for (size_t i = 0; i < strlen; i++) {
                char c = string[strlen - i - 1];
                if (morphine_isblank(c)) {
                    end--;
                } else {
                    break;
                }
            }

            if (end > 0) {
                mapi_push_stringn(U, string, end);
            } else {
                mapi_push_string(U, "");
            }

            maux_nb_return();
    maux_nb_end
}

static void replacefirst(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *find = mapi_get_string(U);
            size_t findlen = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_arg(U, 2);
            maux_expect(U, "string");

            if (findlen > strlen) {
                mapi_pop(U, 1);
                maux_nb_return();
            }

            for (size_t i = 0; i <= strlen - findlen; i++) {
                bool eq = true;
                for (size_t j = 0; j < findlen; j++) {
                    if (string[i + j] != find[j]) {
                        eq = false;
                        break;
                    }
                }

                if (eq) {
                    mapi_push_stringn(U, string, i);
                    mapi_rotate(U, 2);
                    mapi_string_concat(U);
                    mapi_push_stringn(U, string + i + findlen, strlen - findlen);
                    mapi_string_concat(U);
                    break;
                }
            }

            maux_nb_return();
    maux_nb_end
}

static void replacelast(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *find = mapi_get_string(U);
            size_t findlen = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_arg(U, 2);
            maux_expect(U, "string");

            if (findlen > strlen) {
                mapi_pop(U, 1);
                maux_nb_return();
            }

            bool found = false;
            size_t index = 0;
            for (size_t i = 0; i <= strlen - findlen; i++) {
                bool eq = true;
                for (size_t j = 0; j < findlen; j++) {
                    if (string[i + j] != find[j]) {
                        eq = false;
                        break;
                    }
                }

                if (eq) {
                    index = i;
                    found = true;
                }
            }

            if (found) {
                mapi_push_stringn(U, string, index);
                mapi_rotate(U, 2);
                mapi_string_concat(U);
                mapi_push_stringn(U, string + index + findlen, strlen - findlen);
                mapi_string_concat(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void replace(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "string");
            const char *find = mapi_get_string(U);
            size_t findlen = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_arg(U, 2);
            maux_expect(U, "string");

            if (findlen > strlen) {
                mapi_pop(U, 1);
                maux_nb_return();
            }

            bool found = false;
            size_t index = 0;
            for (size_t i = 0; i <= strlen - findlen; i++) {
                bool eq = true;
                for (size_t j = 0; j < findlen; j++) {
                    if (string[i + j] != find[j]) {
                        eq = false;
                        break;
                    }
                }

                if (eq) {
                    found = true;

                    if (index > 0) {
                        mapi_push_stringn(U, string + index, i - index);
                        mapi_string_concat(U);
                        mapi_peek(U, 1);
                        mapi_string_concat(U);
                    } else {
                        mapi_peek(U, 0);
                        mapi_push_stringn(U, string, i);
                        mapi_rotate(U, 2);
                        mapi_string_concat(U);
                    }

                    index += (i - index) + findlen;

                    if (findlen > 0) {
                        i += findlen - 1;
                    }
                }
            }

            if (found && strlen > index) {
                mapi_push_stringn(U, string + index, strlen - index);
                mapi_string_concat(U);
            }

            if (found && strlen > 0 && findlen == 0) {
                mapi_peek(U, 1);
                mapi_rotate(U, 2);
                mapi_string_concat(U);
            }

            if (!found) {
                mapi_pop(U, 1);
            }

            maux_nb_return();
    maux_nb_end
}

struct format_vars {
    enum format_state {
        TEXT = 0,
        FOUND = 1,
        RECOGNIZE = 2,
        GET = 3,
        CONCAT = 4,
    } state;

    size_t index;
    size_t last_index;
    size_t access_index;
};

static struct format_vars load_format_vars(morphine_coroutine_t U) {
    struct format_vars vars;

    maux_localstorage_get(U, "state");
    vars.state = mapi_get_size(U, "state");
    mapi_pop(U, 1);

    maux_localstorage_get(U, "index");
    vars.index = mapi_get_size(U, "index");
    mapi_pop(U, 1);

    maux_localstorage_get(U, "last_index");
    vars.last_index = mapi_get_size(U, "index");
    mapi_pop(U, 1);

    maux_localstorage_get(U, "access_index");
    vars.access_index = mapi_get_size(U, "index");
    mapi_pop(U, 1);

    return vars;
}

static void save_format_vars(morphine_coroutine_t U, struct format_vars vars) {
    mapi_push_size(U, vars.state, "state");
    maux_localstorage_set(U, "state");

    mapi_push_size(U, vars.index, "index");
    maux_localstorage_set(U, "index");

    mapi_push_size(U, vars.last_index, "index");
    maux_localstorage_set(U, "last_index");

    mapi_push_size(U, vars.access_index, "index");
    maux_localstorage_set(U, "access_index");
}

static void format(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            struct format_vars vars = {
                .state = TEXT,
                .index = 0,
                .last_index = 0,
                .access_index = 0
            };

            save_format_vars(U, vars);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            mapi_pop(U, 1);

            mapi_push_string(U, "");
            maux_nb_im_continue(1);
        maux_nb_state(1)
            mapi_push_arg(U, 0);
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);
            mapi_pop(U, 1);

            struct format_vars vars = load_format_vars(U);

            size_t parse_index = 0;
            for (; vars.index < strlen; vars.index++) {
                switch (vars.state) {
                    case TEXT: {
                        if (string[vars.index] == '$') {
                            mapi_push_stringn(U, string + vars.last_index, vars.index - vars.last_index);
                            mapi_string_concat(U);
                            vars.state = FOUND;
                        }
                        break;
                    }
                    case FOUND: {
                        if (string[vars.index] == '{') {
                            parse_index = vars.index + 1;
                            vars.state = RECOGNIZE;
                        } else if (string[vars.index] == '$') {
                            mapi_push_string(U, "$");
                            mapi_string_concat(U);

                            vars.last_index = vars.index + 1;
                            vars.state = TEXT;
                        } else {
                            vars.last_index = vars.index - 1;
                            vars.state = TEXT;
                        }
                        break;
                    }
                    case RECOGNIZE: {
                        if (string[vars.index] != '}') {
                            break;
                        }

                        if (vars.index - parse_index == 0) {
                            mapi_push_size(U, vars.access_index, "index");
                            vars.access_index++;
                        } else {
                            mapi_push_stringn(U, string + parse_index, vars.index - parse_index);
                            vars.access_index++;
                        }

                        mapi_push_arg(U, 1);
                        mapi_rotate(U, 2);

                        vars.state = GET;

                        save_format_vars(U, vars);
                        maux_nb_operation("get", 1);
                    }
                    case GET: {
                        mapi_pop(U, 1);
                        mapi_push_result(U);

                        vars.last_index = vars.index + 1;
                        vars.state = CONCAT;

                        save_format_vars(U, vars);

                        maux_library_access(U, "value.tostr");
                        mapi_rotate(U, 2);
                        maux_nb_call(1, 1);
                    }
                    case CONCAT: {
                        mapi_push_result(U);
                        mapi_string_concat(U);
                        vars.state = TEXT;
                        break;
                    }
                    default:
                        mapi_error(U, "undefined state");
                }
            }

            mapi_push_stringn(U, string + vars.last_index, strlen - vars.last_index);
            mapi_string_concat(U);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("format", format),

    MAUX_CONSTRUCT_FUNCTION("substring", substring),
    MAUX_CONSTRUCT_FUNCTION("trim", trim),
    MAUX_CONSTRUCT_FUNCTION("trimstart", trimstart),
    MAUX_CONSTRUCT_FUNCTION("trimend", trimend),
    MAUX_CONSTRUCT_FUNCTION("split", split),
    MAUX_CONSTRUCT_FUNCTION("replacefirst", replacefirst),
    MAUX_CONSTRUCT_FUNCTION("replacelast", replacelast),
    MAUX_CONSTRUCT_FUNCTION("replace", replace),
    MAUX_CONSTRUCT_FUNCTION("tolower", tolowercase),
    MAUX_CONSTRUCT_FUNCTION("toupper", touppercase),
    MAUX_CONSTRUCT_FUNCTION("repeat", repeat),

    MAUX_CONSTRUCT_FUNCTION("chars", chars),
    MAUX_CONSTRUCT_FUNCTION("codes", codes),
    MAUX_CONSTRUCT_FUNCTION("from", from),

    MAUX_CONSTRUCT_FUNCTION("contains", contains),
    MAUX_CONSTRUCT_FUNCTION("indexof", indexof),
    MAUX_CONSTRUCT_FUNCTION("lastindexof", lastindexof),
    MAUX_CONSTRUCT_FUNCTION("startswith", startswith),
    MAUX_CONSTRUCT_FUNCTION("endswith", endswith),

    MAUX_CONSTRUCT_FUNCTION("isempty", isempty),
    MAUX_CONSTRUCT_FUNCTION("isblank", isblankstr),
    MAUX_CONSTRUCT_FUNCTION("isdigit", isdigitstr),
    MAUX_CONSTRUCT_FUNCTION("isalpha", isalphastr),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_string(void) {
    return (morphine_library_t) {
        .name = "string",
        .sharedkey = NULL,
        .init = library_init
    };
}
