//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/builtin.h"
#include "morphine/utils/ctype.h"

static void substring(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size len = mapi_string_len(U);

            mapi_push_arg(U, 1);
            ml_size start = mapi_get_size(U, "index");

            mapi_push_arg(U, 2);
            ml_size end = mapi_get_size(U, "index");

            mapi_pop(U, 3);

            if (start > len || len < end || start > end) {
                mapi_errorf(
                    U,
                    "cannot sub string with len %"MLIMIT_SIZE_PR" from %"MLIMIT_SIZE_PR" to %"MLIMIT_SIZE_PR,
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
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);

            const char *string = mapi_get_string(U);
            ml_size len = mapi_string_len(U);

            mapi_push_vector_fix(U, len);
            for (ml_size i = 0; i < len; i++) {
                mapi_push_stringn(U, string + i, 1);
                mapi_vector_set(U, i);
            }

            maux_nb_return();
    maux_nb_end
}

static void codes(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);

            const char *string = mapi_get_string(U);
            ml_size len = mapi_string_len(U);

            mapi_push_vector_fix(U, len);
            for (ml_size i = 0; i < len; i++) {
                mapi_push_integer(U, (ml_integer) string[i]);
                mapi_vector_set(U, i);
            }

            maux_nb_return();
    maux_nb_end
}

static void from(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
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

            char *result = mapi_push_userdata_vec(U, size, sizeof(char), NULL, NULL);
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
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            ml_size len = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_boolean(U, len == 0);
            maux_nb_return();
    maux_nb_end
}

#define isctype(n) \
    static void lib_is##n(morphine_coroutine_t U) { \
        maux_nb_function(U) \
            maux_nb_init(); \
                maux_expect_args(U, 1); \
                mapi_push_arg(U, 0); \
                maux_expect(U, mstr_type_string); \
                const char *string = mapi_get_string(U); \
                ml_size len = mapi_string_len(U); \
                mapi_pop(U, 1); \
                bool result = true; \
                for (ml_size i = 0; i < len; i++) { \
                    if (!mm_ctype_is##n(string[i])) { result = false; } \
                } \
                mapi_push_boolean(U, result); \
                maux_nb_return(); \
        maux_nb_end \
    }

isctype(cntrl)
isctype(print)
isctype(space)
isctype(blank)
isctype(graph)
isctype(punct)
isctype(alnum)
isctype(alpha)
isctype(upper)
isctype(lower)
isctype(digit)
isctype(xdigit)

static void repeat(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);

            mapi_push_arg(U, 1);
            ml_size count = mapi_get_size(U, "index");

            mapi_pop(U, 2);

            mapi_push_string(U, "");
            for (ml_size i = 0; i < count; i++) {
                mapi_push_arg(U, 0);

                mapi_string_concat(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void startswith(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *prefix = mapi_get_string(U);
            ml_size plen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool result = strlen >= plen && memcmp(string, prefix, (size_t) plen * sizeof(char)) == 0;

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void endswith(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *suffix = mapi_get_string(U);
            ml_size slen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool result =
                strlen >= slen && memcmp(string + (strlen - slen), suffix, (size_t) slen * sizeof(char)) == 0;

            mapi_push_boolean(U, result);
            maux_nb_return();
    maux_nb_end
}

static void tolowercase(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size len = mapi_string_len(U);

            mapi_pop(U, 1);

            if (len > 0) {
                char *result = mapi_push_userdata_vec(U, len, sizeof(char), NULL, NULL);

                for (ml_size i = 0; i < len; i++) {
                    result[i] = mm_ctype_tolower(string[i]);
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
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size len = mapi_string_len(U);

            mapi_pop(U, 1);

            if (len > 0) {
                char *result = mapi_push_userdata_vec(U, len, sizeof(char), NULL, NULL);

                for (ml_size i = 0; i < len; i++) {
                    result[i] = mm_ctype_toupper(string[i]);
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
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *separator = mapi_get_string(U);
            ml_size seplen = mapi_string_len(U);

            mapi_pop(U, 2);

            mapi_push_vector_dyn(U, 0);
            if (strlen < seplen) {
                maux_nb_return();
            }

            ml_size start = 0;

            if (seplen == 0) {
                mapi_push_string(U, "");
                maux_vector_push(U);
            }

            for (ml_size i = 0; i < strlen; i++) {
                if (i + seplen > strlen) {
                    continue;
                }

                if (seplen == 0) {
                    mapi_push_stringn(U, string + start, 1);
                    maux_vector_push(U);
                    start++;
                } else if (memcmp(string + i, separator, (size_t) seplen * sizeof(char)) == 0) {
                    mapi_push_stringn(U, string + start, i - start);
                    maux_vector_push(U);

                    start = i + seplen;
                    i += seplen - 1;
                }
            }

            if (strlen >= start) {
                mapi_push_stringn(U, string + start, strlen - start);
            } else {
                mapi_push_string(U, "");
            }
            maux_vector_push(U);

            maux_nb_return();
    maux_nb_end
}

static void contains(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *find = mapi_get_string(U);
            ml_size findlen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool found = false;
            if (strlen >= findlen) {
                for (ml_size i = 0; i < strlen - findlen + 1; i++) {
                    if (memcmp(string + i, find, (size_t) findlen * sizeof(char)) == 0) {
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
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *find = mapi_get_string(U);
            ml_size findlen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool found = false;
            ml_size index;
            if (strlen >= findlen) {
                for (ml_size i = 0; i < strlen - findlen + 1; i++) {
                    if (memcmp(string + i, find, findlen * sizeof(char)) == 0) {
                        found = true;
                        index = i;
                        break;
                    }
                }
            }

            if (found) {
                mapi_push_integer(U, index);
            } else {
                mapi_push_nil(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void lastindexof(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *find = mapi_get_string(U);
            ml_size findlen = mapi_string_len(U);

            mapi_pop(U, 2);

            bool found = false;
            ml_size index;
            if (strlen >= findlen) {
                for (ml_size i = 0; i < strlen - findlen + 1; i++) {
                    if (memcmp(string + (strlen - findlen) - i, find, (size_t) findlen * sizeof(char)) == 0) {
                        found = true;
                        index = (strlen - findlen) - i;
                        break;
                    }
                }
            }

            if (found) {
                mapi_push_integer(U, index);
            } else {
                mapi_push_nil(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void trim(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_pop(U, 1);

            ml_size start = 0;
            ml_size end = strlen;

            for (ml_size i = 0; i < strlen; i++) {
                char c = string[i];
                if (mm_ctype_isspace(c)) {
                    start++;
                } else {
                    break;
                }
            }

            for (ml_size i = 0; i < strlen; i++) {
                char c = string[strlen - i - 1];
                if (mm_ctype_isspace(c)) {
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
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_pop(U, 1);

            ml_size start = 0;

            for (ml_size i = 0; i < strlen; i++) {
                char c = string[i];
                if (mm_ctype_isspace(c)) {
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
        maux_nb_init();
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_pop(U, 1);

            ml_size end = strlen;

            for (ml_size i = 0; i < strlen; i++) {
                char c = string[strlen - i - 1];
                if (mm_ctype_isspace(c)) {
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
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *find = mapi_get_string(U);
            ml_size findlen = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_arg(U, 2);
            maux_expect(U, mstr_type_string);

            if (findlen > strlen) {
                mapi_pop(U, 1);
                maux_nb_return();
            }

            for (ml_size i = 0; i <= strlen - findlen; i++) {
                bool eq = true;
                for (ml_size j = 0; j < findlen; j++) {
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
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *find = mapi_get_string(U);
            ml_size findlen = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_arg(U, 2);
            maux_expect(U, mstr_type_string);

            if (findlen > strlen) {
                mapi_pop(U, 1);
                maux_nb_return();
            }

            bool found = false;
            ml_size index = 0;
            for (ml_size i = 0; i <= strlen - findlen; i++) {
                bool eq = true;
                for (ml_size j = 0; j < findlen; j++) {
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
        maux_nb_init();
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, mstr_type_string);
            const char *find = mapi_get_string(U);
            ml_size findlen = mapi_string_len(U);

            mapi_pop(U, 1);

            mapi_push_arg(U, 2);
            maux_expect(U, mstr_type_string);

            if (findlen > strlen) {
                mapi_pop(U, 1);
                maux_nb_return();
            }

            bool found = false;
            ml_size index = 0;
            for (ml_size i = 0; i <= strlen - findlen; i++) {
                bool eq = true;
                for (ml_size j = 0; j < findlen; j++) {
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

    ml_size index;
    ml_size last_index;
    ml_size access_index;
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
    mapi_push_integer(U, vars.state);
    maux_localstorage_set(U, "state");

    mapi_push_integer(U, vars.index);
    maux_localstorage_set(U, "index");

    mapi_push_integer(U, vars.last_index);
    maux_localstorage_set(U, "last_index");

    mapi_push_integer(U, vars.access_index);
    maux_localstorage_set(U, "access_index");
}

static void format(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
            maux_expect_args(U, 2);

            struct format_vars vars = {
                .state = TEXT,
                .index = 0,
                .last_index = 0,
                .access_index = 0
            };

            save_format_vars(U, vars);

            mapi_push_arg(U, 0);
            maux_expect(U, mstr_type_string);
            mapi_pop(U, 1);

            mapi_push_string(U, "");
            maux_nb_im_continue(1);
        maux_nb_state(1);
            mapi_push_arg(U, 0);
            const char *string = mapi_get_string(U);
            ml_size strlen = mapi_string_len(U);
            mapi_pop(U, 1);

            struct format_vars vars = load_format_vars(U);

            ml_size parse_index = 0;
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
                            mapi_push_integer(U, vars.access_index);
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
    MAUX_CONSTRUCT_FUNCTION("iscntrl", lib_iscntrl),
    MAUX_CONSTRUCT_FUNCTION("isprint", lib_isprint),
    MAUX_CONSTRUCT_FUNCTION("isspace", lib_isspace),
    MAUX_CONSTRUCT_FUNCTION("isblank", lib_isblank),
    MAUX_CONSTRUCT_FUNCTION("isgraph", lib_isgraph),
    MAUX_CONSTRUCT_FUNCTION("ispunct", lib_ispunct),
    MAUX_CONSTRUCT_FUNCTION("isalnum", lib_isalnum),
    MAUX_CONSTRUCT_FUNCTION("isalpha", lib_isalpha),
    MAUX_CONSTRUCT_FUNCTION("isupper", lib_isupper),
    MAUX_CONSTRUCT_FUNCTION("islower", lib_islower),
    MAUX_CONSTRUCT_FUNCTION("isdigit", lib_isdigit),
    MAUX_CONSTRUCT_FUNCTION("isxdigit", lib_isxdigit),
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
