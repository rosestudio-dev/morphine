//
// Created by why-iskra on 27.05.2024.
//

#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include "parser.h"
#include "grammar/support/matcher.h"
#include "grammar/support/elements.h"
#include "grammar/impl.h"

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

struct stack {
    size_t type_size;
    size_t expansion_factor;
    size_t limit;
    size_t size;
    size_t used;
    void *array;
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

// stack

#define stack_push_typed(T, U, S)   ((T *) stack_push((U), (S)))
#define stack_peek_typed(T, U, S)   ((T *) stack_peek((U), (S)))
#define stack_get_typed(T, U, S, i) ((T *) stack_get((U), (S), (i)))
#define stack_is_empty(S)           ((S).used == 0)
#define stack_size(S)               ((S).used)

static void stack_init(struct stack *S, size_t type_size, size_t expansion_factor, size_t limit) {
    *S = (struct stack) {
        .type_size = type_size,
        .expansion_factor = expansion_factor,
        .limit = limit,
        .size = 0,
        .used = 0,
        .array = NULL
    };
}

static void stack_free(morphine_instance_t I, struct stack *S) {
    mapi_allocator_free(I, S->array);
}

static void *stack_push(morphine_coroutine_t U, struct stack *S) {
    if (S->used == S->size) {
        if (S->size >= S->limit) {
            mapi_error(U, "stack too big");
        }

        S->array = mapi_allocator_vec(
            mapi_instance(U),
            S->array,
            S->size + S->expansion_factor,
            S->type_size
        );

        S->size += S->expansion_factor;
    }

    void *result = S->array + (S->type_size * S->used);
    S->used++;

    return result;
}

static void stack_pop(morphine_coroutine_t U, struct stack *S, size_t count) {
    if (count > S->used) {
        mapi_error(U, "cannot pop from stack");
    }

    S->used -= count;
}

static void *stack_peek(morphine_coroutine_t U, struct stack *S) {
    if (stack_is_empty(*S)) {
        mapi_error(U, "stack is empty");
    }

    return S->array + (S->type_size * (S->used - 1));
}

static void *stack_get(morphine_coroutine_t U, struct stack *S, size_t index) {
    if (stack_is_empty(*S)) {
        mapi_error(U, "stack is empty");
    }

    if (index >= stack_size(*S)) {
        mapi_error(U, "index is out of bounce");
    }

    return S->array + (S->type_size * index);
}

// debug

static void print_token(morphine_coroutine_t U, struct token token) {
    struct strtable_entry entry;
    switch (token.type) {
        case TT_EOS:
            printf("eos");
            break;
        case TT_INTEGER:
            printf("%d", token.integer);
            break;
        case TT_DECIMAL:
            printf("%g", token.decimal);
            break;
        case TT_STRING:
            mapi_peek(U, 3);
            if (strtable_get_by_index(U, token.string, &entry)) {
                printf("'");
                for (size_t i = 0; i < entry.size; i++) {
                    printf("%c", entry.string[i]);
                }
                printf("'");
            } else {
                printf("ERROR");
            }
            mapi_pop(U, 1);
            break;
        case TT_WORD:
            mapi_peek(U, 3);
            if (strtable_get_by_index(U, token.word, &entry)) {
                for (size_t i = 0; i < entry.size; i++) {
                    printf("%c", entry.string[i]);
                }
            } else {
                printf("ERROR");
            }
            mapi_pop(U, 1);
            break;
        case TT_PREDEFINED_WORD:
            printf("%s", lex_predefined2str(U, token.predefined_word));
            break;
        case TT_OPERATOR:
            printf("%s", lex_operator2str(U, token.operator));
            break;
    }
}

static void print_reduce(enum reduce_type type) {
    switch (type) {
        case REDUCE_TYPE_AST:
            printf("AST");
            return;
        case REDUCE_TYPE_EXPRESSION:
            printf("EXPRESSION");
            return;
        case REDUCE_TYPE_ADDITIVE:
            printf("ADDITIVE");
            return;
        case REDUCE_TYPE_MULTIPLICATIVE:
            printf("MULTIPLICATIVE");
            return;
        case REDUCE_TYPE_VALUE:
            printf("VALUE");
            return;
        case REDUCE_TYPE_OR:
            printf("OR");
            return;
        case REDUCE_TYPE_AND:
            printf("AND");
            return;
        case REDUCE_TYPE_EQUAL:
            printf("EQUAL");
            return;
        case REDUCE_TYPE_CONDITION:
            printf("CONDITION");
            return;
        case REDUCE_TYPE_CONCAT:
            printf("CONCAT");
            return;
        case REDUCE_TYPE_PREFIX:
            printf("PREFIX");
            return;
        case REDUCE_TYPE_POSTFIX:
            printf("POSTFIX");
            return;
        case REDUCE_TYPE_TABLE:
            printf("TABLE");
            return;
        case REDUCE_TYPE_VECTOR:
            printf("VECTOR");
            return;
        case REDUCE_TYPE_VARIABLE:
            printf("VARIABLE");
            return;
        case REDUCE_TYPE_STATEMENT:
            printf("STATEMENT");
            return;
        case REDUCE_TYPE_STATEMENT_BLOCK:
            printf("STATEMENT_BLOCK");
            return;
        case REDUCE_TYPE_BLOCK_ELEM:
            printf("BLOCK_ELEM");
            return;
        case REDUCE_TYPE_WHILE:
            printf("WHILE");
            return;
        case REDUCE_TYPE_DO_WHILE:
            printf("DO_WHILE");
            return;
        case REDUCE_TYPE_FOR:
            printf("FOR");
            return;
        case REDUCE_TYPE_FUNCTION:
            printf("FUNCTION");
            return;
        case REDUCE_TYPE_DECLARATION:
            printf("DECLARATION");
            return;
        case REDUCE_TYPE_ASSIGMENT:
            printf("ASSIGMENT");
            return;
        case REDUCE_TYPE_EXPRESSION_BLOCK:
            printf("EXPRESSION_BLOCK");
            return;
        case REDUCE_TYPE_PRIMARY:
            printf("PRIMARY");
            return;
        case REDUCE_TYPE_IMPLICIT_BLOCK_ELEM:
            printf("IMPLICIT_BLOCK_ELEM");
            return;
        case REDUCE_TYPE_ITERATOR:
            printf("ITERATOR");
            return;
        case REDUCE_TYPE_STATEMENT_IF:
            printf("STATEMENT_IF");
            return;
        case REDUCE_TYPE_EXPRESSION_IF:
            printf("EXPRESSION_IF");
            return;
    }

    printf("UNDEFINED");
}

static void print_stack(morphine_coroutine_t U, struct parser *P, const char *prefix, size_t pos) {
    if (prefix != NULL) {
        printf("%s | ", prefix);
    }

    for (size_t i = 0; i < P->elements.used; i++) {
        struct element element = *stack_get_typed(struct element, U, &P->elements, i);

        if (i == pos) {
            printf("{");
        }

        if (element.is_token) {
            print_token(U, element.token);
        } else {
            print_reduce(element.reduce.type);
        }

        if (i == pos) {
            printf("}");
        }

        printf(" ");
    }
    printf("\n");
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
        print_stack(M->U, M->P, "s", M->context.from + M->position - 1);
        return true;
    }

    return false;
}

struct token matcher_consume(struct matcher *M, struct matcher_symbol symbol) {
    struct token token;
    if (matcher_get_token(M, &token) && matcher_symbol(symbol, token)) {
        matcher_push(M);
        print_stack(M->U, M->P, "s", M->context.from + M->position - 1);
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

        print_stack(U, P, "u", stack_size(P->elements) - 1);
    } else {
        print_stack(U, P, "r", stack_size(P->elements) - 1);
    }

    return true;
}
