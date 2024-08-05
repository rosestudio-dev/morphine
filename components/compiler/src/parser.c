//
// Created by why-iskra on 05.08.2024.
//

#include <setjmp.h>
#include <stdio.h>
#include "morphinec/parser.h"
#include "grammar/controller.h"

#define LIMIT_ELEMENTS   131072
#define EXPANSION_FACTOR 32

struct element {
    bool is_reduced;
    struct mc_lex_token token;

    struct {
        parse_function_t function;
        struct mc_ast_node *node;
    } reduced;
};

struct context {
    size_t from;
    parse_function_t function;

    struct context *prev;
};

struct mc_parser {
    struct {
        bool has;
        struct mc_lex_token token;
    } lookahead;

    struct {
        size_t size;
        size_t allocated;
        struct element *elements;
    } stack;

    struct {
        struct context *current;
        struct context *trash;
    } context;
};

struct parse_controller {
    morphine_coroutine_t U;
    struct mc_parser *P;
    struct mc_ast *A;
    struct mc_lex *L;
    struct mc_strtable *T;

    size_t position;
    jmp_buf reduce_jump;
    struct {
        parse_function_t function;
    } reduce_data;
};

// functions

//static void print_stack(morphine_coroutine_t U, struct mc_parser *P) {
//    for (size_t i = 0; i < P->stack.size; i++) {
//        struct context *current = P->context.current;
//        while (current != NULL) {
//            if (current->from == i) {
//                printf("| ");
//            }
//
//            current = current->prev;
//        }
//
//        struct element element = P->stack.elements[i];
//        if (element.is_reduced) {
//            printf("[%s]", mcapi_ast_type_name(U, element.reduced.node));
//        } else {
//            switch (element.token.type) {
//                case MCLTT_EOS:
//                    printf("EOS");
//                    break;
//                case MCLTT_INTEGER:
//                    printf("INT");
//                    break;
//                case MCLTT_DECIMAL:
//                    printf("DEC");
//                    break;
//                case MCLTT_STRING:
//                    printf("STR");
//                    break;
//                case MCLTT_WORD:
//                    printf("WRD");
//                    break;
//                case MCLTT_PREDEFINED_WORD:
//                    printf("%s", mcapi_lex_predefined2str(U, element.token.predefined_word));
//                    break;
//                case MCLTT_OPERATOR:
//                    printf("%s", mcapi_lex_operator2name(U, element.token.operator));
//                    break;
//                case MCLTT_COMMENT:
//                    printf("CMM");
//                    break;
//            }
//        }
//
//        printf(" ");
//    }
//    printf("\n");
//}

static void push_element(
    morphine_coroutine_t U,
    struct mc_parser *P,
    struct element element
) {
    if (P->stack.size == P->stack.allocated) {
        if (P->stack.allocated >= LIMIT_ELEMENTS) {
            mapi_error(U, "parse limit is reached");
        }

        size_t new_size = P->stack.allocated + EXPANSION_FACTOR;
        P->stack.elements = mapi_allocator_vec(
            mapi_instance(U),
            P->stack.elements,
            new_size,
            sizeof(struct element)
        );

        P->stack.allocated = new_size;
    }

    P->stack.elements[P->stack.size] = element;
    P->stack.size++;
}

static void pop_element(
    morphine_coroutine_t U,
    struct mc_parser *P,
    size_t size
) {
    if (size > P->stack.size) {
        mapi_error(U, "parse pop is out of bounce");
    }

    P->stack.size -= size;
}

static void push_context(
    morphine_coroutine_t U,
    struct mc_parser *P,
    parse_function_t function
) {
    struct context *context;
    if (P->context.trash == NULL) {
        context = mapi_allocator_uni(mapi_instance(U), NULL, sizeof(struct context));
    } else {
        context = P->context.trash;
        P->context.trash = context->prev;
    }

    *context = (struct context) {
        .from = P->stack.size,
        .function = function,
        .prev = P->context.current
    };

    P->context.current = context;
}

static void pop_context(
    morphine_coroutine_t U,
    struct mc_parser *P
) {
    if (P->context.current == NULL) {
        mapi_error(U, "cannot pop parse context");
    }

    struct context *context = P->context.current;
    P->context.current = context->prev;

    context->prev = P->context.trash;
    P->context.trash = context;
}

static struct mc_lex_token next_token(
    morphine_coroutine_t U,
    struct mc_lex *L,
    struct mc_strtable *T
) {
    struct mc_lex_token token;
    do {
        token = mcapi_lex_step(U, L, T);
    } while (token.type == MCLTT_COMMENT);

    return token;
}

static void update_lookahead(struct parse_controller *C) {
    if (!C->P->lookahead.has) {
        C->P->lookahead.has = true;
        C->P->lookahead.token = next_token(C->U, C->L, C->T);
    }
}

static struct element current_element(struct parse_controller *C, bool *need_push) {
    size_t position = C->P->context.current->from + C->position;
    if (position > C->P->stack.size) {
        mapi_error(C->U, "parser stack is corrupted");
    } else if (position == C->P->stack.size) {
        update_lookahead(C);

        if (need_push != NULL) {
            *need_push = true;
        }

        return (struct element) {
            .is_reduced = false,
            .token = C->P->lookahead.token
        };
    }

    if (need_push != NULL) {
        *need_push = false;
    }

    return C->P->stack.elements[position];
}

static void shift(struct parse_controller *C, struct element element, bool need_push) {
    C->position++;

    if (need_push) {
        push_element(C->U, C->P, element);
        C->P->lookahead.has = false;
    }
}

static bool token_soft_eq(struct mc_lex_token token, struct mc_lex_token test) {
    switch (token.type) {
        case MCLTT_EOS:
        case MCLTT_INTEGER:
        case MCLTT_DECIMAL:
        case MCLTT_STRING:
        case MCLTT_WORD:
        case MCLTT_COMMENT:
            return test.type == token.type;
        case MCLTT_PREDEFINED_WORD:
            return test.type == token.type && test.predefined_word == token.predefined_word;
        case MCLTT_OPERATOR:
            return test.type == token.type && test.operator == token.operator;
    }

    return false;
}

// control

morphine_coroutine_t parser_U(struct parse_controller *C) {
    return C->U;
}

struct mc_ast *parser_A(struct parse_controller *C) {
    return C->A;
}

ml_line parser_get_line(struct parse_controller *C) {
    struct element element = current_element(C, NULL);
    if (element.is_reduced) {
        return element.reduced.node->line;
    } else {
        return element.token.line;
    }
}

morphine_noret void parser_error(struct parse_controller *C, const char *text) {
    mapi_errorf(C->U, "line %"MLIMIT_LINE_PR": %s", parser_get_line(C), text);
}

bool parser_look(struct parse_controller *C, struct mc_lex_token expected) {
    struct element element = current_element(C, NULL);
    return token_soft_eq(expected, element.token);
}

bool parser_match(struct parse_controller *C, struct mc_lex_token expected) {
    bool need_push = false;
    struct element element = current_element(C, &need_push);
    if (!element.is_reduced && token_soft_eq(expected, element.token)) {
        shift(C, element, need_push);
        return true;
    }

    return false;
}

struct mc_lex_token parser_consume(struct parse_controller *C, struct mc_lex_token expected) {
    bool need_push = false;
    struct element element = current_element(C, &need_push);
    if (!element.is_reduced && token_soft_eq(expected, element.token)) {
        shift(C, element, need_push);
        return element.token;
    }

    const char *name = "???";
    switch (expected.type) {
        case MCLTT_EOS:
            name = "eos";
            break;
        case MCLTT_INTEGER:
            name = "integer";
            break;
        case MCLTT_DECIMAL:
            name = "decimal";
            break;
        case MCLTT_STRING:
            name = "text";
            break;
        case MCLTT_WORD:
            name = "word";
            break;
        case MCLTT_COMMENT:
            name = "comment";
            break;
        case MCLTT_PREDEFINED_WORD:
            name = mcapi_lex_predefined2str(C->U, expected.predefined_word);
            break;
        case MCLTT_OPERATOR:
            name = mcapi_lex_operator2str(C->U, expected.operator);
            break;
    }

    mapi_errorf(C->U, "line %"MLIMIT_LINE_PR": expected %s", parser_get_line(C), name);
}

struct mc_ast_node *parser_reduce(struct parse_controller *C, parse_function_t function) {
    size_t position = C->P->context.current->from + C->position;
    if (position > C->P->stack.size) {
        mapi_error(C->U, "parser stack is corrupted");
    } else if (position == C->P->stack.size) {
        C->reduce_data.function = function;
        longjmp(C->reduce_jump, 1);
    }

    struct element element = C->P->stack.elements[position];

    if (!element.is_reduced || element.reduced.function != function) {
        mapi_error(C->U, "parser reduce is corrupted");
    }

    C->position++;

    return element.reduced.node;
}

void parser_reset(struct parse_controller *C) {
    C->position = 0;
}

// api

static void parser_userdata_init(morphine_instance_t I, void *data) {
    (void) I;

    struct mc_parser *P = data;
    *P = (struct mc_parser) {
        .lookahead.has = false,
        .stack.size = 0,
        .stack.allocated = 0,
        .stack.elements = NULL,
        .context.current = NULL,
        .context.trash = NULL,
    };
}

static void parser_userdata_free_context(morphine_instance_t I, struct context *pool) {
    struct context *context = pool;
    while (context != NULL) {
        struct context *prev = context->prev;
        mapi_allocator_free(I, context);
        context = prev;
    }
}

static void parser_userdata_free(morphine_instance_t I, void *data) {
    struct mc_parser *P = data;
    mapi_allocator_free(I, P->stack.elements);
    parser_userdata_free_context(I, P->context.current);
    parser_userdata_free_context(I, P->context.trash);
}

MORPHINE_API struct mc_parser *mcapi_push_parser(morphine_coroutine_t U) {
    mapi_type_declare(
        mapi_instance(U),
        MC_PARSER_USERDATA_TYPE,
        sizeof(struct mc_parser),
        parser_userdata_init,
        parser_userdata_free,
        false
    );

    struct mc_parser *P = mapi_push_userdata(U, MC_PARSER_USERDATA_TYPE);
    push_context(U, P, parse_root);

    return P;
}

MORPHINE_API struct mc_parser *mcapi_get_parser(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, MC_PARSER_USERDATA_TYPE);
}

MORPHINE_API bool mcapi_parser_step(
    morphine_coroutine_t U,
    struct mc_parser *P,
    struct mc_ast *A,
    struct mc_lex *L,
    struct mc_strtable *T
) {
    if (P->context.current == NULL) {
        return false;
    }

    struct parse_controller controller = {
        .U = U,
        .P = P,
        .A = A,
        .L = L,
        .T = T,
        .position = 0,
        .reduce_data.function = NULL
    };

    if (setjmp(controller.reduce_jump) != 0) {
        push_context(U, P, controller.reduce_data.function);
        return true;
    }

    struct element current = current_element(&controller, NULL);

    struct element element = {
        .is_reduced = true,
        .token = current.token,
        .reduced.function = P->context.current->function,
        .reduced.node = P->context.current->function(&controller)
    };
    pop_element(U, P, controller.position);
    pop_context(U, P);

    push_element(U, P, element);
    return true;
}
