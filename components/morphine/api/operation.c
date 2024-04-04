//
// Created by whyiskra on 1/16/24.
//

#include <string.h>
#include "morphine/api.h"
#include "morphine/core/operations.h"
#include "morphine/object/coroutine/stack/access.h"
#include "morphine/object/coroutine/stack/call.h"

struct op_func {
    const char *name;
    bool (*function)(morphine_coroutine_t U);
};

static bool opiterator(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_iterator(
        U, callstackI_state(U),
        args[0], args + 1,
        0, false
    ) == CALLED;

    return result;
}

static bool opiteratorinit(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 1);

    bool result = interpreter_fun_iterator_init(
        U, callstackI_state(U),
        args[0],
        0, false
    ) == CALLED;

    return result;
}

static bool opiteratorhas(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_iterator_has(
        U, callstackI_state(U),
        args[0], args + 1,
        0, false
    ) == CALLED;

    return result;
}

static bool opiteratornext(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_iterator_next(
        U, callstackI_state(U),
        args[0], args + 1,
        0, false
    ) == CALLED;

    return result;
}

static bool opget(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_get(
        U, callstackI_state(U),
        args[0], args[1], args + 1,
        0, false
    ) == CALLED;

    return result;
}

static bool opset(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 3);

    bool result = interpreter_fun_set(
        U, callstackI_state(U),
        args[0], args[1], args[2],
        2, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 2);
    }

    return result;
}

static bool opadd(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_add(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opsub(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_sub(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opmul(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_mul(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opdiv(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_div(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opmod(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_mod(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opequal(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_equal(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opless(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_less(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool oplesseq(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_less_equal(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opand(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_and(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opor(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_or(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool opconcat(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 2);

    bool result = interpreter_fun_concat(
        U, callstackI_state(U),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(U, 1);
    }

    return result;
}

static bool optype(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 1);

    bool result = interpreter_fun_type(
        U, callstackI_state(U),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opneg(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 1);

    bool result = interpreter_fun_negative(
        U, callstackI_state(U),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opnot(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 1);

    bool result = interpreter_fun_not(
        U, callstackI_state(U),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool oplen(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 1);

    bool result = interpreter_fun_length(
        U, callstackI_state(U),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opref(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 1);

    bool result = interpreter_fun_ref(
        U, callstackI_state(U),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opderef(morphine_coroutine_t U) {
    struct value *args = stackI_vector(U, 0, 1);

    bool result = interpreter_fun_deref(
        U, callstackI_state(U),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

MORPHINE_API bool mapi_op(morphine_coroutine_t U, const char *op) {
    struct op_func ops[] = {
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
        (struct op_func) { .name = "lesseq", .function = oplesseq },
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

    size_t size = sizeof(ops) / sizeof(struct op_func);
    for (size_t i = 0; i < size; i++) {
        if (strcmp(ops[i].name, op) == 0) {
            return ops[i].function(U);
        }
    }

    throwI_errorf(U->I, "Undefined %s operation", op);
}
