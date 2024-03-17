//
// Created by whyiskra on 1/16/24.
//

#include <string.h>
#include "morphine/api.h"
#include "morphine/core/call.h"
#include "morphine/core/operations.h"

struct op_func {
    const char *name;
    bool (*function)(morphine_state_t S);
};

static bool opget(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_get(
        S, callI_callstate(S),
        args[0], args[1], args + 1,
        0, false
    ) == CALLED;

    return result;
}

static bool opset(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 3);

    bool result = interpreter_fun_set(
        S, callI_callstate(S),
        args[0], args[1], args[2],
        2, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 2);
    }

    return result;
}

static bool opadd(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_add(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opsub(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_sub(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opmul(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_mul(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opdiv(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_div(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opmod(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_mod(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opequal(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_equal(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opless(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_less(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool oplesseq(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_less_equal(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opand(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_and(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opor(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_or(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool opconcat(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 2);

    bool result = interpreter_fun_concat(
        S, callI_callstate(S),
        args[0], args[1], args,
        1, false
    ) == CALLED;

    if (!result) {
        stackI_pop(S, 1);
    }

    return result;
}

static bool optype(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 1);

    bool result = interpreter_fun_type(
        S, callI_callstate(S),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opneg(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 1);

    bool result = interpreter_fun_negative(
        S, callI_callstate(S),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opnot(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 1);

    bool result = interpreter_fun_not(
        S, callI_callstate(S),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool oplen(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 1);

    bool result = interpreter_fun_length(
        S, callI_callstate(S),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opref(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 1);

    bool result = interpreter_fun_ref(
        S, callI_callstate(S),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

static bool opderef(morphine_state_t S) {
    struct value *args = stackI_vector(S, 0, 1);

    bool result = interpreter_fun_deref(
        S, callI_callstate(S),
        args[0], args,
        0, false
    ) == CALLED;

    return result;
}

MORPHINE_API bool mapi_op(morphine_state_t S, const char *op) {
    struct op_func ops[] = {
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
            return ops[i].function(S);
        }
    }

    throwI_errorf(S, "Undefined %s operation", op);
}
