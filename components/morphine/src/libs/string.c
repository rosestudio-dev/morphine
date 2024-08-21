//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include <ctype.h>
#include "morphine/libs/builtin.h"

static inline bool checkblank(char c) {
    return isspace(c);
}

static inline bool checkdigit(char c) {
    return isdigit(c);
}

static inline bool checkalpha(char c) {
    return isalpha(c);
}

static void substring(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "index");
            size_t start = mapi_get_size(U);

            mapi_push_arg(U, 2);
            maux_expect(U, "index");
            size_t end = mapi_get_size(U);

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

static void codeat(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "index");
            size_t at = mapi_get_size(U);

            mapi_pop(U, 2);

            if (at >= len) {
                mapi_errorf(U, "cannot get code of char at %zu position", at);
            }

            mapi_push_integer(U, string[at]);
            maux_nb_return();
    maux_nb_end
}

static void charat(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "index");
            size_t at = mapi_get_size(U);

            mapi_pop(U, 2);

            if (at >= len) {
                mapi_errorf(U, "cannot get char at %zu position", at);
            }

            mapi_push_stringf(U, "%c", string[at]);
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
                if (!checkblank(string[i])) {
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
                if (!checkdigit(string[i])) {
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
                if (!checkalpha(string[i])) {
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
            maux_expect(U, "size");
            size_t count = mapi_get_size(U);

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

            bool result = strlen >= plen && memcmp(string, prefix, plen) == 0;

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

            bool result = strlen >= slen && memcmp(string + (strlen - slen), suffix, slen) == 0;

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
                    result[i] = (char) tolower(string[i]);
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
                    result[i] = (char) toupper(string[i]);
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
                } else if (memcmp(string + i, separator, seplen) == 0) {
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
                    if (memcmp(string + i, find, findlen) == 0) {
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
                    if (memcmp(string + i, find, findlen) == 0) {
                        found = true;
                        index = i;
                        break;
                    }
                }
            }

            if (found) {
                mapi_push_size(U, index);
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
                    if (memcmp(string + (strlen - findlen) - i, find, findlen) == 0) {
                        found = true;
                        index = (strlen - findlen) - i;
                        break;
                    }
                }
            }

            if (found) {
                mapi_push_size(U, index);
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
                if (checkblank(c)) {
                    start++;
                } else {
                    break;
                }
            }

            for (size_t i = 0; i < strlen; i++) {
                char c = string[strlen - i - 1];
                if (checkblank(c)) {
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
                if (checkblank(c)) {
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
                if (checkblank(c)) {
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

static void format(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *string = mapi_get_string(U);
            size_t strlen = mapi_string_len(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "table");

            bool found = false;
            size_t parsed = 0;
            size_t index = 0;
            for (size_t i = 0; i < strlen; i++) {
                if (string[i] != '$') {
                    continue;
                }

                found = true;

                size_t offset = 0;
                if (i + 1 < strlen && string[i + 1] == '$') {
                    if (index > 0) {
                        mapi_rotate(U, 2);
                    }

                    mapi_push_string(U, "$");

                    offset = 1;
                } else if (i + 1 < strlen && string[i + 1] == '{') {
                    size_t c = i + 2;
                    bool closed = false;
                    for (; c < strlen; c++) {
                        if (string[c] == '}') {
                            closed = true;
                            break;
                        }
                    }

                    if (!closed) {
                        mapi_errorf(U, "format access key isn't closed");
                    }

                    if (index > 0) {
                        mapi_rotate(U, 2);
                        mapi_push_stringn(U, string + i + 2, c - i - 2);
                        mapi_table_get(U);
                        mapi_to_string(U);
                    } else {
                        mapi_push_stringn(U, string + i + 2, c - i - 2);
                        mapi_table_get(U);
                        mapi_to_string(U);
                    }

                    offset = c - i;
                } else {
                    if (index > 0) {
                        mapi_rotate(U, 2);
                        mapi_push_size(U, parsed);
                        mapi_table_get(U);
                        mapi_to_string(U);
                    } else {
                        mapi_push_size(U, parsed);
                        mapi_table_get(U);
                        mapi_to_string(U);
                    }

                    parsed++;
                }

                if (index > 0) {
                    mapi_peek(U, 2);
                    mapi_push_stringn(U, string + index, i - index);
                    mapi_string_concat(U);
                    mapi_rotate(U, 2);
                    mapi_string_concat(U);
                } else {
                    mapi_push_stringn(U, string, i);
                    mapi_rotate(U, 2);
                    mapi_string_concat(U);
                }

                index = i + offset + 1;
                i += offset;
            }

            if (found && strlen > index) {
                mapi_push_stringn(U, string + index, strlen - index);
                mapi_string_concat(U);
            }

            if (!found) {
                mapi_pop(U, 1);
            }

            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "format",       format },

    { "substring",    substring },
    { "trim",         trim },
    { "trimstart",    trimstart },
    { "trimend",      trimend },
    { "split",        split },
    { "replacefirst", replacefirst },
    { "replacelast",  replacelast },
    { "replace",      replace },
    { "tolower",      tolowercase },
    { "toupper",      touppercase },
    { "repeat",       repeat },

    { "chars",        chars },
    { "codeat",       codeat },
    { "charat",       charat },

    { "contains",     contains },
    { "indexof",      indexof },
    { "lastindexof",  lastindexof },
    { "startswith",   startswith },
    { "endswith",     endswith },

    { "isempty",      isempty },
    { "isblank",      isblankstr },
    { "isdigit",      isdigitstr },
    { "isalpha",      isalphastr },

    { NULL, NULL },
};

static morphine_library_t library = {
    .name = "string",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_string(void) {
    return &library;
}
