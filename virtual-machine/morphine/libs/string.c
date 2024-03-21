//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include <ctype.h>
#include "morphine/core/libloaders.h"

static inline bool checkblank(char c) {
    return isspace(c);
}

static void substring(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,integer,integer", "string,integer,integer");

            const char *string;
            size_t len;
            size_t start;
            size_t end;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                len = mapi_string_len(S);

                mapi_push_arg(S, 0);
                start = mapi_get_size(S);
                mapi_push_arg(S, 1);
                end = mapi_get_size(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                len = mapi_string_len(S);

                mapi_push_arg(S, 1);
                start = mapi_get_size(S);
                mapi_push_arg(S, 2);
                end = mapi_get_size(S);
            }

            mapi_pop(S, 3);

            if (start > len || len < end || start > end) {
                mapi_errorf(
                    S,
                    "Cannot sub string with len %zu from %zu to %zu",
                    len,
                    start,
                    end
                );
            }

            mapi_push_stringn(S, string + start, end - start);
            nb_return();
    nb_end
}

static void tochararray(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string", "string");

            if (variant == 0) {
                mapi_push_self(S);
            } else {
                mapi_push_arg(S, 0);
            }

            mapi_pop(S, 1);

            const char *string = mapi_get_string(S);
            size_t len = mapi_string_len(S);

            mapi_push_table(S, len);

            for (size_t i = 0; i < len; i++) {
                mapi_push_size(S, i);
                mapi_push_stringf(S, "%c", string[i]);
                mapi_table_set(S);
            }

            nb_return();
    nb_end
}

static void codeat(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,integer", "string,integer");

            const char *string;
            size_t len;
            size_t at;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                len = mapi_string_len(S);

                mapi_push_arg(S, 0);
                at = mapi_get_size(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                len = mapi_string_len(S);

                mapi_push_arg(S, 1);
                at = mapi_get_size(S);
            }

            mapi_pop(S, 2);

            if (at >= len) {
                mapi_errorf(S, "Cannot get code of char at %zu position", at);
            }

            mapi_push_integer(S, string[at]);
            nb_return();
    nb_end
}

static void charat(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,integer", "string,integer");

            const char *string;
            size_t len;
            size_t at;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                len = mapi_string_len(S);

                mapi_push_arg(S, 0);
                at = mapi_get_size(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                len = mapi_string_len(S);

                mapi_push_arg(S, 1);
                at = mapi_get_size(S);
            }

            mapi_pop(S, 2);

            if (at >= len) {
                mapi_errorf(S, "Cannot get char at %zu position", at);
            }

            mapi_push_stringf(S, "%c", string[at]);
            nb_return();
    nb_end
}

static void isempty(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string", "string");

            size_t len;

            if (variant == 0) {
                mapi_push_self(S);
                len = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                len = mapi_string_len(S);
            }

            mapi_pop(S, 1);

            mapi_push_boolean(S, len == 0);
            nb_return();
    nb_end
}

static void isblankstr(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string", "string");

            const char *string;
            size_t len;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                len = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                len = mapi_string_len(S);
            }

            mapi_pop(S, 1);

            bool isblank = true;

            for (size_t i = 0; i < len; i++) {
                if (!checkblank(string[i])) {
                    isblank = false;
                }
            }

            mapi_push_boolean(S, isblank);
            nb_return();
    nb_end
}

static void repeat(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,integer", "string,integer");

            size_t count;

            if (variant == 0) {
                mapi_push_arg(S, 0);
                count = mapi_get_size(S);
            } else {
                mapi_push_arg(S, 1);
                count = mapi_get_size(S);
            }

            mapi_pop(S, 1);

            mapi_push_string(S, "");
            for (size_t i = 0; i < count; i++) {
                if (variant == 0) {
                    mapi_push_self(S);
                } else {
                    mapi_push_arg(S, 0);
                }

                mapi_string_concat(S);
            }

            nb_return();
    nb_end
}

static void startswith(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *prefix;
            size_t plen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 0);
                prefix = mapi_get_string(S);
                plen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 1);
                prefix = mapi_get_string(S);
                plen = mapi_string_len(S);
            }

            mapi_pop(S, 2);

            bool result = strlen >= plen && memcmp(string, prefix, plen) == 0;

            mapi_push_boolean(S, result);
            nb_return();
    nb_end
}

static void endswith(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *suffix;
            size_t slen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 0);
                suffix = mapi_get_string(S);
                slen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 1);
                suffix = mapi_get_string(S);
                slen = mapi_string_len(S);
            }

            mapi_pop(S, 2);

            bool result = strlen >= slen && memcmp(string + (strlen - slen), suffix, slen) == 0;

            mapi_push_boolean(S, result);
            nb_return();
    nb_end
}

static void tolowercase(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string", "string");

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);
            }

            mapi_pop(S, 1);

            if(strlen > 0) {
                char *result = mapi_allocator_uni(mapi_instance(S), NULL, strlen);
                mapi_push_userdata(S, "tempstring", result, NULL, mapi_allocator_free);

                for (size_t i = 0; i < strlen; i++) {
                    result[i] = (char) tolower(string[i]);
                }

                mapi_push_stringn(S, result, strlen);
            } else {
                mapi_push_string(S, "");
            }

            nb_return();
    nb_end
}

static void touppercase(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string", "string");

            const char *string;
            size_t strlen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);
            }

            mapi_pop(S, 1);

            if(strlen > 0) {
                char *result = mapi_allocator_uni(mapi_instance(S), NULL, strlen);
                mapi_push_userdata(S, "tempstring", result, NULL, mapi_allocator_free);

                for (size_t i = 0; i < strlen; i++) {
                    result[i] = (char) toupper(string[i]);
                }

                mapi_push_stringn(S, result, strlen);
            } else {
                mapi_push_string(S, "");
            }

            nb_return();
    nb_end
}

static void split(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *separator;
            size_t seplen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 0);
                separator = mapi_get_string(S);
                seplen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 1);
                separator = mapi_get_string(S);
                seplen = mapi_string_len(S);
            }

            mapi_pop(S, 2);

            mapi_push_table(S, 0);
            if (strlen < seplen) {
                nb_return();
            }

            size_t count = 0;
            size_t start = 0;

            if (seplen == 0) {
                mapi_push_size(S, count);
                mapi_push_string(S, "");
                mapi_table_set(S);
                count++;
            }

            for (size_t i = 0; i < strlen; i++) {
                if (i + seplen > strlen) {
                    continue;
                }

                if (seplen == 0) {
                    mapi_push_size(S, count);
                    mapi_push_stringn(S, string + start, 1);
                    mapi_table_set(S);
                    count++;
                    start++;
                } else if (memcmp(string + i, separator, seplen) == 0) {
                    mapi_push_size(S, count);
                    mapi_push_stringn(S, string + start, i - start);
                    mapi_table_set(S);

                    count++;
                    start = i + seplen;
                    i += seplen - 1;
                }
            }

            mapi_push_size(S, count);
            if (strlen >= start) {
                mapi_push_stringn(S, string + start, strlen - start);
            } else {
                mapi_push_string(S, "");
            }
            mapi_table_set(S);

            nb_return();
    nb_end
}

static void contains(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 0);
                find = mapi_get_string(S);
                findlen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 1);
                find = mapi_get_string(S);
                findlen = mapi_string_len(S);
            }

            mapi_pop(S, 2);

            bool found = false;
            if (strlen >= findlen) {
                for (size_t i = 0; i < strlen - findlen + 1; i++) {
                    if (memcmp(string + i, find, findlen) == 0) {
                        found = true;
                        break;
                    }
                }
            }

            mapi_push_boolean(S, found);
            nb_return();
    nb_end
}

static void indexof(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 0);
                find = mapi_get_string(S);
                findlen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 1);
                find = mapi_get_string(S);
                findlen = mapi_string_len(S);
            }

            mapi_pop(S, 2);

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
                mapi_push_size(S, index);
            } else {
                mapi_push_nil(S);
            }
            nb_return();
    nb_end
}

static void lastindexof(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs_pattern(S, 3, "self:string,string", "string,string");

            const char *string;
            size_t strlen;

            const char *find;
            size_t findlen;

            if (variant == 0) {
                mapi_push_self(S);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 0);
                find = mapi_get_string(S);
                findlen = mapi_string_len(S);
            } else {
                mapi_push_arg(S, 0);
                string = mapi_get_string(S);
                strlen = mapi_string_len(S);

                mapi_push_arg(S, 1);
                find = mapi_get_string(S);
                findlen = mapi_string_len(S);
            }

            mapi_pop(S, 2);

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
                mapi_push_size(S, index);
            } else {
                mapi_push_nil(S);
            }
            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
//    { "format", NULL },

    { "substring",   substring },
//    { "trim",        NULL },
//    { "trimstart",   NULL },
//    { "trimend",     NULL },
    { "split",       split },
//    { "replace",     NULL },
//    { "replacelast", NULL },
//    { "replaceall",  NULL },
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

    { NULL,     NULL },
};

void mlib_string_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_string_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
