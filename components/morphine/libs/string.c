//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include <ctype.h>
#include "morphine/libs/loader.h"

static inline bool checkblank(char c) {
    return isspace(c);
}

static void substring(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(
                U,
                2,
                "self:string,integer,integer",
                "string,integer,integer"
            );

            const char *string;
            size_t len;
            size_t start;
            size_t end;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                len = mapi_string_len(U);

                mapi_push_arg(U, 0);
                start = mapi_get_size(U);

                mapi_push_arg(U, 1);
                end = mapi_get_size(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                len = mapi_string_len(U);

                mapi_push_arg(U, 1);
                start = mapi_get_size(U);

                mapi_push_arg(U, 2);
                end = mapi_get_size(U);
            }

            mapi_pop(U, 3);

            if (start > len || len < end || start > end) {
                mapi_errorf(
                    U,
                    "Cannot sub string with len %zu from %zu to %zu",
                    len,
                    start,
                    end
                );
            }

            mapi_push_stringn(U, string + start, end - start);
            nb_return();
    nb_end
}

static void tochararray(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            if (variant == 0) {
                mapi_push_self(U);
            } else {
                mapi_push_arg(U, 0);
            }

            mapi_pop(U, 1);

            const char *string = mapi_get_string(U);
            size_t len = mapi_string_len(U);

            mapi_push_table(U);

            for (size_t i = 0; i < len; i++) {
                mapi_push_size(U, i);
                mapi_push_stringf(U, "%c", string[i]);
                mapi_table_set(U);
            }

            nb_return();
    nb_end
}

static void codeat(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,integer", "string,integer");

            const char *string;
            size_t len;
            size_t at;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                len = mapi_string_len(U);

                mapi_push_arg(U, 0);
                at = mapi_get_size(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                len = mapi_string_len(U);

                mapi_push_arg(U, 1);
                at = mapi_get_size(U);
            }

            mapi_pop(U, 2);

            if (at >= len) {
                mapi_errorf(U, "Cannot get code of char at %zu position", at);
            }

            mapi_push_integer(U, string[at]);
            nb_return();
    nb_end
}

static void charat(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,integer", "string,integer");

            const char *string;
            size_t len;
            size_t at;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                len = mapi_string_len(U);

                mapi_push_arg(U, 0);
                at = mapi_get_size(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                len = mapi_string_len(U);

                mapi_push_arg(U, 1);
                at = mapi_get_size(U);
            }

            mapi_pop(U, 2);

            if (at >= len) {
                mapi_errorf(U, "Cannot get char at %zu position", at);
            }

            mapi_push_stringf(U, "%c", string[at]);
            nb_return();
    nb_end
}

static void isempty(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            size_t len;

            if (variant == 0) {
                mapi_push_self(U);
                len = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                len = mapi_string_len(U);
            }

            mapi_pop(U, 1);

            mapi_push_boolean(U, len == 0);
            nb_return();
    nb_end
}

static void isblankstr(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            const char *string;
            size_t len;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                len = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                len = mapi_string_len(U);
            }

            mapi_pop(U, 1);

            bool isblank = true;

            for (size_t i = 0; i < len; i++) {
                if (!checkblank(string[i])) {
                    isblank = false;
                }
            }

            mapi_push_boolean(U, isblank);
            nb_return();
    nb_end
}

static void repeat(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,integer", "string,integer");

            size_t count;

            if (variant == 0) {
                mapi_push_arg(U, 0);
                count = mapi_get_size(U);
            } else {
                mapi_push_arg(U, 1);
                count = mapi_get_size(U);
            }

            mapi_pop(U, 1);

            mapi_push_string(U, "");
            for (size_t i = 0; i < count; i++) {
                if (variant == 0) {
                    mapi_push_self(U);
                } else {
                    mapi_push_arg(U, 0);
                }

                mapi_string_concat(U);
            }

            nb_return();
    nb_end
}

static void startswith(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *prefix;
            size_t plen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                prefix = mapi_get_string(U);
                plen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                prefix = mapi_get_string(U);
                plen = mapi_string_len(U);
            }

            mapi_pop(U, 2);

            bool result = strlen >= plen && memcmp(string, prefix, plen) == 0;

            mapi_push_boolean(U, result);
            nb_return();
    nb_end
}

static void endswith(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *suffix;
            size_t slen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                suffix = mapi_get_string(U);
                slen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                suffix = mapi_get_string(U);
                slen = mapi_string_len(U);
            }

            mapi_pop(U, 2);

            bool result = strlen >= slen && memcmp(string + (strlen - slen), suffix, slen) == 0;

            mapi_push_boolean(U, result);
            nb_return();
    nb_end
}

static void tolowercase(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            }

            mapi_pop(U, 1);

            if (strlen > 0) {
                char *result = mapi_allocator_uni(mapi_instance(U), NULL, strlen * sizeof(char));
                mapi_push_userdata(U, "tempstring", result, NULL, mapi_allocator_free);

                for (size_t i = 0; i < strlen; i++) {
                    result[i] = (char) tolower(string[i]);
                }

                mapi_push_stringn(U, result, strlen);
            } else {
                mapi_push_string(U, "");
            }

            nb_return();
    nb_end
}

static void touppercase(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            }

            mapi_pop(U, 1);

            if (strlen > 0) {
                char *result = mapi_allocator_uni(mapi_instance(U), NULL, strlen * sizeof(char));
                mapi_push_userdata(U, "tempstring", result, NULL, mapi_allocator_free);

                for (size_t i = 0; i < strlen; i++) {
                    result[i] = (char) toupper(string[i]);
                }

                mapi_push_stringn(U, result, strlen);
            } else {
                mapi_push_string(U, "");
            }

            nb_return();
    nb_end
}

static void split(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *separator;
            size_t seplen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                separator = mapi_get_string(U);
                seplen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                separator = mapi_get_string(U);
                seplen = mapi_string_len(U);
            }

            mapi_pop(U, 2);

            mapi_push_table(U);
            if (strlen < seplen) {
                nb_return();
            }

            size_t count = 0;
            size_t start = 0;

            if (seplen == 0) {
                mapi_push_size(U, count);
                mapi_push_string(U, "");
                mapi_table_set(U);
                count++;
            }

            for (size_t i = 0; i < strlen; i++) {
                if (i + seplen > strlen) {
                    continue;
                }

                if (seplen == 0) {
                    mapi_push_size(U, count);
                    mapi_push_stringn(U, string + start, 1);
                    mapi_table_set(U);
                    count++;
                    start++;
                } else if (memcmp(string + i, separator, seplen) == 0) {
                    mapi_push_size(U, count);
                    mapi_push_stringn(U, string + start, i - start);
                    mapi_table_set(U);

                    count++;
                    start = i + seplen;
                    i += seplen - 1;
                }
            }

            mapi_push_size(U, count);
            if (strlen >= start) {
                mapi_push_stringn(U, string + start, strlen - start);
            } else {
                mapi_push_string(U, "");
            }
            mapi_table_set(U);

            nb_return();
    nb_end
}

static void contains(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);
            }

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
            nb_return();
    nb_end
}

static void indexof(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);
            }

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
            nb_return();
    nb_end
}

static void lastindexof(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);
            }

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
            nb_return();
    nb_end
}

static void trim(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            }

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

            nb_return();
    nb_end
}

static void trimstart(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            }

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

            nb_return();
    nb_end
}

static void trimend(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "self:string", "string");

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);
            }

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

            nb_return();
    nb_end
}

static void replace(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(
                U, 2, "self:string,string,string", "string,string,string"
            );

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            const char *replace;
            size_t replen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                replace = mapi_get_string(U);
                replen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);

                mapi_push_arg(U, 2);
                replace = mapi_get_string(U);
                replen = mapi_string_len(U);
            }

            mapi_pop(U, 2);

            if (findlen > strlen) {
                nb_return();
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
                    size_t len = strlen + (replen - findlen);
                    char *result = mapi_allocator_uni(
                        mapi_instance(U), NULL, len * sizeof(char)
                    );

                    mapi_push_userdata(
                        U, "tempstring", result, NULL, mapi_allocator_free
                    );

                    memcpy(result, string, i);
                    memcpy(result + i, replace, replen);
                    if (strlen > i + findlen) {
                        memcpy(
                            result + i + replen,
                            string + i + findlen,
                            strlen - (i + findlen)
                        );
                    }

                    mapi_push_stringn(U, result, len);
                    break;
                }
            }

            nb_return();
    nb_end
}

static void replacelast(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(
                U, 2, "self:string,string,string", "string,string,string"
            );

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            const char *replace;
            size_t replen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                replace = mapi_get_string(U);
                replen = mapi_string_len(U);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);

                mapi_push_arg(U, 2);
                replace = mapi_get_string(U);
                replen = mapi_string_len(U);
            }

            mapi_pop(U, 2);

            if (findlen > strlen) {
                nb_return();
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
                size_t len = strlen + (replen - findlen);
                char *result = mapi_allocator_uni(
                    mapi_instance(U), NULL, len * sizeof(char)
                );

                mapi_push_userdata(
                    U, "tempstring", result, NULL, mapi_allocator_free
                );

                memcpy(result, string, index);
                memcpy(result + index, replace, replen);
                if (strlen > index + findlen) {
                    memcpy(
                        result + index + replen,
                        string + index + findlen,
                        strlen - (index + findlen)
                    );
                }

                mapi_push_stringn(U, result, len);
            }

            nb_return();
    nb_end
}

static void replaceall(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(
                U, 2, "self:string,string,string", "string,string,string"
            );

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);

                mapi_pop(U, 1);

                mapi_push_arg(U, 1);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
                find = mapi_get_string(U);
                findlen = mapi_string_len(U);

                mapi_pop(U, 1);

                mapi_push_arg(U, 2);
            }

            if (findlen > strlen) {
                mapi_pop(U, 1);
                nb_return();
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

            nb_return();
    nb_end
}

static void format(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(
                U, 2, "self:string,table", "string,table"
            );

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(U);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 0);
            } else {
                mapi_push_arg(U, 0);
                string = mapi_get_string(U);
                strlen = mapi_string_len(U);

                mapi_push_arg(U, 1);
            }

            bool found = false;
            ml_integer parsed = 0;
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
                        mapi_errorf(U, "Format access key isn't closed");
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
                        mapi_push_integer(U, parsed);
                        mapi_table_get(U);
                        mapi_to_string(U);
                    } else {
                        mapi_push_integer(U, parsed);
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

            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "format",      format },

    { "substring",   substring },
    { "trim",        trim },
    { "trimstart",   trimstart },
    { "trimend",     trimend },
    { "split",       split },
    { "replace",     replace },
    { "replacelast", replacelast },
    { "replaceall",  replaceall },
    { "tolower",     tolowercase },
    { "toupper",     touppercase },
    { "repeat",      repeat },

    { "tochararray", tochararray },
    { "codeat",      codeat },
    { "charat",      charat },

    { "contains",    contains },
    { "indexof",     indexof },
    { "lastindexof", lastindexof },
    { "startswith",  startswith },
    { "endswith",    endswith },

    { "isempty",     isempty },
    { "isblank",     isblankstr },

    { NULL, NULL },
};

void mlib_string_loader(morphine_coroutine_t U) {
    maux_construct(U, table);
}

MORPHINE_LIB void mlib_string_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, name, argc);
}
