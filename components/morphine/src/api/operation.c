//
// Created by whyiskra on 1/16/24.
//

#include "morphine/api.h"
#include "morphine/core/operations.h"
#include "morphine/utils/array_size.h"
#include <string.h>

struct op_func {
    const char *name;
    bool (*function)(morphine_coroutine_t U);
};

#define unary_op(name) \
    static bool op##name(morphine_coroutine_t U) { \
        struct value a = stackI_peek(U, 0); \
        struct value result_value = valueI_nil; \
        bool result = interpreter_fun_##name(U, callstackI_state(U), a, &result_value, 1, false) == CALLED; \
        if (!result) { stackI_replace(U, 0, result_value); } \
        return result; \
    }

#define binary_op(name) \
    static bool op##name(morphine_coroutine_t U) { \
        struct value a = stackI_peek(U, 1); \
        struct value b = stackI_peek(U, 0); \
        struct value result_value = valueI_nil; \
        bool result = interpreter_fun_##name(U, callstackI_state(U), a, b, &result_value, 1, false) == CALLED; \
        if (!result) { stackI_replace(U, 1, result_value); stackI_pop(U, 1); } \
        return result; \
    }

static bool opget(morphine_coroutine_t U) {
    struct value container = stackI_peek(U, 1);
    struct value key = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_get(U, callstackI_state(U), container, key, &result_value, 1, false) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opset(morphine_coroutine_t U) {
    struct value container = stackI_peek(U, 2);
    struct value key = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    bool result = interpreter_fun_set(U, callstackI_state(U), container, key, value, 2, false) == CALLED;

    if (!result) {
        stackI_pop(U, 2);
    }

    return result;
}

binary_op(add);
binary_op(sub);
binary_op(mul);
binary_op(div);
binary_op(mod);
binary_op(equal);
binary_op(less);
binary_op(and);
binary_op(or);
binary_op(concat);

unary_op(type);
unary_op(negative);
unary_op(not);
unary_op(length);

unary_op(tostr);
binary_op(compare);
unary_op(hash);

static struct op_func ops[] = {
    (struct op_func) { .name = "get",          .function = opget          },
    (struct op_func) { .name = "set",          .function = opset          },
    (struct op_func) { .name = "add",          .function = opadd          },
    (struct op_func) { .name = "sub",          .function = opsub          },
    (struct op_func) { .name = "mul",          .function = opmul          },
    (struct op_func) { .name = "div",          .function = opdiv          },
    (struct op_func) { .name = "mod",          .function = opmod          },
    (struct op_func) { .name = "equal",        .function = opequal        },
    (struct op_func) { .name = "less",         .function = opless         },
    (struct op_func) { .name = "and",          .function = opand          },
    (struct op_func) { .name = "or",           .function = opor           },
    (struct op_func) { .name = "concat",       .function = opconcat       },
    (struct op_func) { .name = "type",         .function = optype         },
    (struct op_func) { .name = "neg",          .function = opnegative     },
    (struct op_func) { .name = "not",          .function = opnot          },
    (struct op_func) { .name = "len",          .function = oplength       },
    (struct op_func) { .name = "tostr",        .function = optostr        },
    (struct op_func) { .name = "compare",      .function = opcompare      },
    (struct op_func) { .name = "hash",         .function = ophash         },
};

MORPHINE_API bool mapi_op(morphine_coroutine_t U, const char *op) {
    for (size_t i = 0; i < array_size(ops); i++) {
        if (strcmp(ops[i].name, op) == 0) {
            return ops[i].function(U);
        }
    }

    throwI_errorf(U->I, "undefined %s operation", op);
}
