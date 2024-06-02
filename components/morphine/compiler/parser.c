//
// Created by why-iskra on 27.05.2024.
//

#include <string.h>
#include <setjmp.h>
#include "morphine/compiler/parser.h"
#include "grammar/support/matcher.h"
#include "grammar/support/elements.h"
#include "grammar/impl.h"
#include "stack.h"

#define MORPHINE_TYPE "parser"

#define LIMIT_STACK_CONTEXTS   262144
#define LIMIT_STACK_ELEMENTS   262144
#define EXPANSION_FACTOR_STACK 16

struct element {
    bool is_token;

    union {
        struct token token;
        struct reduce reduce;
    };
};

struct elements {
    morphine_coroutine_t U;
    size_t size;
    struct element *array;
};

struct context {
    bool is_wrapped;
    size_t from;
    enum reduce_type type;
};

struct parser {
    struct {
        bool has;
        struct token token;
    } lookahead;

    struct stack contexts;
    struct stack elements;
};

struct matcher {
    morphine_coroutine_t U;
    struct parser *P;
    struct context context;
    size_t position;
    jmp_buf push_context_jump;
    enum reduce_type push_context_reduce_type;
};

// element

size_t elements_size(struct elements *E) {
    return E->size;
}

static struct element elements_get(struct elements *E, size_t index) {
    if (index >= E->size) {
        mapi_error(E->U, "element index is out of bounce");
    }

    return E->array[index];
}

uint32_t elements_line(struct elements *E, size_t index) {
    struct element element = elements_get(E, index);
    if (element.is_token) {
        return element.token.line;
    } else {
        return element.reduce.node->line;
    }
}

morphine_noret void elements_error(struct elements *E, size_t index, const char *message) {
    uint32_t line;
    struct element element = elements_get(E, index);
    if (element.is_token) {
        line = element.token.line;
    } else {
        line = element.reduce.node->line;
    }

    mapi_errorf(E->U, "line %"PRIu32": %s", line, message);
}

bool elements_is_token(struct elements *E, size_t index) {
    return elements_get(E, index).is_token;
}

struct token elements_get_token(struct elements *E, size_t index) {
    struct element element = elements_get(E, index);

    if (!element.is_token) {
        mapi_error(E->U, "element isn't token");
    }

    return element.token;
}

struct reduce elements_get_reduce(struct elements *E, size_t index) {
    struct element element = elements_get(E, index);

    if (element.is_token) {
        mapi_error(E->U, "element isn't reduce");
    }

    return element.reduce;
}

bool elements_look(struct elements *E, size_t index, struct matcher_symbol symbol) {
    if (!elements_is_token(E, index)) {
        return false;
    }

    struct token token = elements_get_token(E, index);

    return matcher_symbol(symbol, token);
}

// initialization

static void parser_free(morphine_instance_t I, void *p) {
    struct parser *P = p;

    stack_free(I, &P->contexts);
    stack_free(I, &P->elements);
}

static struct parser *get_parser(morphine_coroutine_t U) {
    mapi_peek(U, 1);
    if (strcmp(mapi_userdata_type(U), MORPHINE_TYPE) == 0) {
        struct parser *P = mapi_userdata_pointer(U);
        mapi_pop(U, 1);
        return P;
    } else {
        mapi_error(U, "expected "MORPHINE_TYPE);
    }
}

void parser(morphine_coroutine_t U) {
    struct parser *P = mapi_push_userdata(U, MORPHINE_TYPE, sizeof(struct parser));

    *P = (struct parser) {
        .lookahead.has = false
    };

    stack_init(&P->contexts, sizeof(struct context), EXPANSION_FACTOR_STACK, LIMIT_STACK_CONTEXTS);
    stack_init(&P->elements, sizeof(struct element), EXPANSION_FACTOR_STACK, LIMIT_STACK_ELEMENTS);

    mapi_userdata_set_free(U, parser_free);

    *stack_push_typed(struct context, U, &P->contexts) = (struct context) {
        .is_wrapped = false,
        .from = 0,
        .type = REDUCE_TYPE_AST
    };

    ast(U);
}

// matcher

bool matcher_symbol(struct matcher_symbol symbol, struct token token) {
    switch (symbol.type) {
        case TT_EOS:
        case TT_INTEGER:
        case TT_DECIMAL:
        case TT_STRING:
        case TT_WORD:
            return symbol.type == token.type;
        case TT_PREDEFINED_WORD:
            return symbol.type == token.type && symbol.predefined_word == token.predefined_word;
        case TT_OPERATOR:
            return symbol.type == token.type && symbol.operator == token.operator;
    }

    return false;
}

static size_t matcher_get_stack_position(struct matcher *M) {
    size_t stack_position = M->context.from + M->position;
    if (stack_size(M->P->elements) < stack_position) {
        mapi_error(M->U, "stack corrupted");
    }

    return stack_position;
}

static bool matcher_get_token(struct matcher *M, struct token *result) {
    size_t stack_position = matcher_get_stack_position(M);

    struct token token;
    if (stack_size(M->P->elements) == stack_position) {
        if (M->P->lookahead.has) {
            token = M->P->lookahead.token;
        } else {
            mapi_peek(M->U, 3);
            mapi_peek(M->U, 3);
            token = lex_next(M->U);
            mapi_pop(M->U, 2);
        }

        M->P->lookahead.token = token;
        M->P->lookahead.has = true;
    } else {
        struct element element = *stack_get_typed(
            struct element,
            M->U,
            &M->P->elements,
            stack_position
        );

        if (!element.is_token) {
            return false;
        }

        token = element.token;
    }

    if (result != NULL) {
        *result = token;
    }

    return true;
}

static void matcher_push(struct matcher *M) {
    size_t stack_position = matcher_get_stack_position(M);

    M->position++;

    if (stack_size(M->P->elements) != stack_position) {
        return;
    }

    struct token token;
    if (M->P->lookahead.has) {
        M->P->lookahead.has = false;
        token = M->P->lookahead.token;
    } else {
        mapi_peek(M->U, 3);
        mapi_peek(M->U, 3);
        token = lex_next(M->U);
        mapi_pop(M->U, 2);
    }

    *stack_push_typed(struct element, M->U, &M->P->elements) = (struct element) {
        .is_token = true,
        .token = token
    };
}

static uint32_t matcher_get_line(struct matcher *M) {
    struct token token;
    if (matcher_get_token(M, &token)) {
        return token.line;
    } else {
        size_t stack_position = matcher_get_stack_position(M);

        struct element element = *stack_get_typed(
            struct element,
            M->U,
            &M->P->elements,
            stack_position
        );

        return element.reduce.node->line;
    }
}

morphine_noret void matcher_error(struct matcher *M, const char *message) {
    mapi_errorf(M->U, "line %"PRIu32": %s", matcher_get_line(M), message);
}

bool matcher_look(struct matcher *M, struct matcher_symbol symbol) {
    struct token token;
    if (matcher_get_token(M, &token)) {
        return matcher_symbol(symbol, token);
    }

    return false;
}

bool matcher_match(struct matcher *M, struct matcher_symbol symbol) {
    if (matcher_look(M, symbol)) {
        matcher_push(M);
        return true;
    }

    return false;
}

struct token matcher_consume(struct matcher *M, struct matcher_symbol symbol) {
    struct token token;
    if (matcher_get_token(M, &token) && matcher_symbol(symbol, token)) {
        matcher_push(M);
        return token;
    }

    const char *name;

    switch (symbol.type) {
        case TT_EOS:
            name = "eos";
            break;
        case TT_INTEGER:
            name = "integer";
            break;
        case TT_DECIMAL:
            name = "decimal";
            break;
        case TT_STRING:
            name = "text";
            break;
        case TT_WORD:
            name = "word";
            break;
        case TT_PREDEFINED_WORD:
            name = lex_predefined2str(M->U, symbol.predefined_word);
            break;
        case TT_OPERATOR:
            name = lex_operator2str(M->U, symbol.operator);
            break;
        default:
            name = "???";
            break;
    }

    mapi_errorf(M->U, "line %"PRIu32": expected %s", matcher_get_line(M), name);
}

void matcher_reduce(struct matcher *M, enum reduce_type type) {
    size_t stack_position = matcher_get_stack_position(M);

    if (stack_size(M->P->elements) == stack_position) {
        M->push_context_reduce_type = type;
        longjmp(M->push_context_jump, 1);
    }

    struct element element = *stack_get_typed(
        struct element,
        M->U,
        &M->P->elements,
        stack_position
    );

    if (element.is_token || element.reduce.type != type) {
        mapi_errorf(M->U, "line %"PRIu32": reduce error", matcher_get_line(M));
    }

    M->position++;
}

bool matcher_is_reduced(struct matcher *M, enum reduce_type type) {
    size_t stack_position = matcher_get_stack_position(M);

    if (stack_size(M->P->elements) == stack_position) {
        return false;
    }

    struct element element = *stack_get_typed(
        struct element,
        M->U,
        &M->P->elements,
        stack_position
    );

    return !element.is_token && element.reduce.type == type;
}

// parse

static struct grammar_quantum get_grammar_quantum(morphine_coroutine_t U, struct context context) {
    for (size_t i = 0; i < grammar_size(); i++) {
        if (grammar[i].type == context.type) {
            return grammar[i];
        }
    }

    mapi_error(U, "rule not found");
}

bool parser_next(morphine_coroutine_t U) {
    struct parser *P = get_parser(U);

    if (stack_is_empty(P->contexts)) {
        struct element element = *stack_get_typed(struct element, U, &P->elements, 0);
        if (element.is_token) {
            mapi_error(U, "parse error");
        }

        ast_ready(U, element.reduce.node);
        return false;
    }

    struct context context = *stack_peek_typed(struct context, U, &P->contexts);
    struct grammar_quantum grammar_quantum = get_grammar_quantum(U, context);

    struct matcher matcher = {
        .U = U,
        .P = P,
        .context = context,
        .position = context.is_wrapped ? 1 : 0,
    };

    if (setjmp(matcher.push_context_jump) != 0) {
        *stack_push_typed(struct context, U, &P->contexts) = (struct context) {
            .is_wrapped = false,
            .from = stack_size(P->elements),
            .type = matcher.push_context_reduce_type
        };

        return true;
    }

    bool push_recursion = false;
    uint32_t line = matcher_get_line(&matcher);

    if (grammar_quantum.is_wrapping) {
        push_recursion = grammar_quantum.wrapping(&matcher, context.is_wrapped);
    } else {
        grammar_quantum.normal(&matcher);
    }

    struct elements elements = {
        .U = U,
        .array = stack_get_typed(struct element, U, &P->elements, context.from),
        .size = stack_size(P->elements) - context.from
    };

    struct ast_node *node = grammar_quantum.assemble(U, &elements);

    if (node == NULL) {
        mapi_errorf(U, "line %"PRIu32": error while assemble ast node", line);
    }

    stack_pop(U, &P->elements, matcher.position);
    stack_pop(U, &P->contexts, 1);

    *stack_push_typed(struct element, U, &P->elements) = (struct element) {
        .is_token = false,
        .reduce.type = context.type,
        .reduce.node = node
    };

    if (push_recursion) {
        *stack_push_typed(struct context, U, &P->contexts) = (struct context) {
            .is_wrapped = true,
            .from = stack_size(P->elements) - 1,
            .type = context.type
        };
    }

    return true;
}
