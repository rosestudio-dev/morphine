//
// Created by whyiskra on 1/16/24.
//

#include <string.h>
#include "morphine/api.h"
#include "morphine/core/operations.h"
#include "morphine/core/stack.h"
#include "morphine/core/throw.h"

struct op_func {
    const char *name;
    bool (*function)(morphine_coroutine_t U);
};

static bool opiterator(morphine_coroutine_t U) {
    struct value container = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_iterator(
        U, callstackI_state(U),
        container, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_push(U, result_value);
    }

    return result;
}

static bool opiteratorinit(morphine_coroutine_t U) {
    struct value iterator = stackI_peek(U, 2);
    struct value key_name = stackI_peek(U, 1);
    struct value value_name = stackI_peek(U, 0);

    op_result_t result = interpreter_fun_iterator_init(
        U, callstackI_state(U),
        iterator, key_name, value_name,
        2, false
    );

    if (result == NORMAL) {
        stackI_pop(U, 2);
    }

    return result == CALLED;
}

static bool opiteratorhas(morphine_coroutine_t U) {
    struct value iterator = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_iterator_has(
        U, callstackI_state(U),
        iterator, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_push(U, result_value);
    }

    return result;
}

static bool opiteratornext(morphine_coroutine_t U) {
    struct value iterator = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_iterator_next(
        U, callstackI_state(U),
        iterator, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_push(U, result_value);
    }

    return result;
}

static bool opget(morphine_coroutine_t U) {
    struct value container = stackI_peek(U, 1);
    struct value key = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_get(
        U, callstackI_state(U),
        container, key, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opset(morphine_coroutine_t U) {
    struct value container = stackI_peek(U, 2);
    struct value key = stackI_peek(U, 1);
    struct value value = stackI_peek(U, 0);

    op_result_t result = interpreter_fun_set(
        U, callstackI_state(U),
        container, key, value,
        2, false
    );

    if (result == NORMAL) {
        stackI_pop(U, 2);
    }

    return result == CALLED;
}

static bool opadd(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_add(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opsub(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_sub(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opmul(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_mul(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opdiv(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_div(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opmod(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_mod(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opequal(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_equal(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opless(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_less(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opand(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_and(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opor(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_or(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opconcat(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_concat(
        U, callstackI_state(U),
        a, b, &result_value,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool optype(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_type(
        U, callstackI_state(U),
        a, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opneg(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_negative(
        U, callstackI_state(U),
        a, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opnot(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_not(
        U, callstackI_state(U),
        a, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool oplen(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_length(
        U, callstackI_state(U),
        a, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opref(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_ref(
        U, callstackI_state(U),
        a, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}

static bool opderef(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 0);

    struct value result_value = valueI_nil;
    bool result = interpreter_fun_deref(
        U, callstackI_state(U),
        a, &result_value,
        0, false
    ) == CALLED;

    if (!result) {
        stackI_replace(U, 0, result_value);
    }

    return result;
}


static struct op_func ops[] = {
    (struct op_func) { .name = "iterator", .function = opiterator },
    (struct op_func) { .name = "iteratorinit", .function = opiteratorinit },
    (struct op_func) { .name = "iteratorhas", .function = opiteratorhas },
    (struct op_func) { .name = "iteratornext", .function = opiteratornext },
    (struct op_func) { .name = "get", .function = opget },
    (struct op_func) { .name = "set", .function = opset },
    (struct op_func) { .name = "add", .function = opadd },
    (struct op_func) { .name = "sub", .function = opsub },
    (struct op_func) { .name = "mul", .function = opmul },
    (struct op_func) { .name = "div", .function = opdiv },
    (struct op_func) { .name = "mod", .function = opmod },
    (struct op_func) { .name = "equal", .function = opequal },
    (struct op_func) { .name = "less", .function = opless },
    (struct op_func) { .name = "and", .function = opand },
    (struct op_func) { .name = "or", .function = opor },
    (struct op_func) { .name = "concat", .function = opconcat },
    (struct op_func) { .name = "type", .function = optype },
    (struct op_func) { .name = "neg", .function = opneg },
    (struct op_func) { .name = "not", .function = opnot },
    (struct op_func) { .name = "len", .function = oplen },
    (struct op_func) { .name = "ref", .function = opref },
    (struct op_func) { .name = "deref", .function = opderef },
};

MORPHINE_API bool mapi_op(morphine_coroutine_t U, const char *op) {
    size_t size = sizeof(ops) / sizeof(struct op_func);
    for (size_t i = 0; i < size; i++) {
        if (strcmp(ops[i].name, op) == 0) {
            return ops[i].function(U);
        }
    }

    throwI_errorf(U->I, "undefined %s operation", op);
}
