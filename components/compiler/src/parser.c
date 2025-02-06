//
// Created by why-iskra on 05.08.2024.
//

#include <setjmp.h>
#include <string.h>
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

    enum predefined_word_mode predefined_word_mode;
    size_t position;
    jmp_buf reduce_jump;
    struct {
        parse_function_t function;
    } reduce_data;
};

struct predefined_word_entry {
    const char *name;
    union {
        enum predefined_word_normal normal_type;
        enum predefined_word_asm asm_type;
    };
};

static struct predefined_word_entry predefined_normal_table[] = {
#define predef_word(n) { .normal_type = PWN_##n, .name = #n },

#include "grammar/predefword/normal.h"

#undef predef_word
};

static struct predefined_word_entry predefined_asm_table[] = {
#define predef_word(n) { .asm_type = PWA_##n, .name = #n },

#include "grammar/predefword/asm.h"

#undef predef_word
};

// functions

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
        .prev = P->context.current,
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
    } while (token.type == MCLTT_COMMENT ||
             token.type == MCLTT_MULTILINE_COMMENT);

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

static const char *predefined2str(struct parse_controller *C, struct predefined_word predefined_word) {
    size_t size;
    struct predefined_word_entry *table;
    switch (predefined_word.mode) {
        case PWM_NORMAL: {
            size = sizeof(predefined_normal_table) / sizeof(predefined_normal_table[0]);
            table = predefined_normal_table;
            break;
        }
        case PWM_ASM: {
            size = sizeof(predefined_asm_table) / sizeof(predefined_asm_table[0]);
            table = predefined_asm_table;
            break;
        }
        default:
            mapi_error(C->U, "unsupported predefined word mode");
    }

    for (size_t i = 0; i < size; i++) {
        switch (predefined_word.mode) {
            case PWM_NORMAL:
                if (table[i].normal_type == predefined_word.normal_type) {
                    return table[i].name;
                }
                break;
            case PWM_ASM:
                if (table[i].asm_type == predefined_word.asm_type) {
                    return table[i].name;
                }
                break;
        }
    }

    mapi_error(C->U, "wrong predefined word");
}

static bool is_predefined(
    struct parse_controller *C,
    struct predefined_word predefined_word,
    mc_strtable_index_t word
) {
    if (C->predefined_word_mode != predefined_word.mode) {
        return false;
    }

    size_t size;
    struct predefined_word_entry *table;
    switch (C->predefined_word_mode) {
        case PWM_NORMAL: {
            size = sizeof(predefined_normal_table) / sizeof(predefined_normal_table[0]);
            table = predefined_normal_table;
            break;
        }
        case PWM_ASM: {
            size = sizeof(predefined_asm_table) / sizeof(predefined_asm_table[0]);
            table = predefined_asm_table;
            break;
        }
        default:
            mapi_error(C->U, "unsupported predefined word mode");
    }

    struct mc_strtable_entry entry = mcapi_strtable_access(C->U, C->T, word);
    for (size_t i = 0; i < size; i++) {
        if (strlen(table[i].name) != entry.size) {
            continue;
        }

        if (memcmp(table[i].name, entry.string, entry.size * sizeof(char)) == 0) {
            switch (C->predefined_word_mode) {
                case PWM_NORMAL:
                    return table[i].normal_type == predefined_word.normal_type;
                case PWM_ASM:
                    return table[i].asm_type == predefined_word.asm_type;
            }
        }
    }

    return false;
}

static bool is_not_predefined(
    struct parse_controller *C,
    mc_strtable_index_t word
) {
    size_t size;
    struct predefined_word_entry *table;
    switch (C->predefined_word_mode) {
        case PWM_NORMAL: {
            size = sizeof(predefined_normal_table) / sizeof(predefined_normal_table[0]);
            table = predefined_normal_table;
            break;
        }
        case PWM_ASM: {
            size = sizeof(predefined_asm_table) / sizeof(predefined_asm_table[0]);
            table = predefined_asm_table;
            break;
        }
        default:
            mapi_error(C->U, "unsupported predefined word mode");
    }

    struct mc_strtable_entry entry = mcapi_strtable_access(C->U, C->T, word);
    for (size_t i = 0; i < size; i++) {
        if (strlen(table[i].name) != entry.size) {
            continue;
        }

        if (memcmp(table[i].name, entry.string, entry.size * sizeof(char)) == 0) {
            return false;
        }
    }

    return true;
}

static bool is_expected_token(
    struct parse_controller *C,
    struct expected_token expected,
    struct mc_lex_token token
) {
    switch (expected.type) {
        case ETT_INTEGER:
            return token.type == MCLTT_INTEGER;
        case ETT_DECIMAL:
            return token.type == MCLTT_DECIMAL;
        case ETT_STRING:
            return token.type == MCLTT_STRING;
        case ETT_WORD:
            return token.type == MCLTT_EXTENDED_WORD ||
                   (token.type == MCLTT_WORD && is_not_predefined(C, token.word));
        case ETT_EOS:
            return token.type == MCLTT_EOS;
        case ETT_OPERATOR:
            return token.type == MCLTT_OPERATOR && expected.op == token.op;
        case ETT_PREDEFINED_WORD:
            return token.type == MCLTT_WORD && is_predefined(C, expected.predefined_word, token.word);
        case ETT_IMPLICIT_WORD:
            return token.type == MCLTT_WORD;
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
    }

    return element.token.line;
}

ml_size parser_index(struct parse_controller *C) {
    struct element element = current_element(C, NULL);
    if(element.is_reduced) {
        return element.reduced.node->from;
    }

    return element.token.index;
}

mattr_noret void parser_errorf(struct parse_controller *C, const char *str, ...) {
    va_list args;
    va_start(args, str);
    mapi_push_stringv(C->U, str, args);
    va_end(args);

    mapi_errorf(
        C->U,
        "line %"MLIMIT_LINE_PR": %s",
        parser_get_line(C),
        mapi_get_string(C->U)
    );
}

struct mc_strtable_entry parser_string(struct parse_controller *C, mc_strtable_index_t index) {
    return mcapi_strtable_access(C->U, C->T, index);
}

bool parser_look(struct parse_controller *C, struct expected_token expected) {
    struct element element = current_element(C, NULL);
    return is_expected_token(C, expected, element.token);
}

bool parser_match(struct parse_controller *C, struct expected_token expected) {
    bool need_push = false;
    struct element element = current_element(C, &need_push);
    if (!element.is_reduced && is_expected_token(C, expected, element.token)) {
        shift(C, element, need_push);
        return true;
    }

    return false;
}

struct mc_lex_token parser_consume(struct parse_controller *C, struct expected_token expected) {
    bool need_push = false;
    struct element element = current_element(C, &need_push);
    if (!element.is_reduced && is_expected_token(C, expected, element.token)) {
        shift(C, element, need_push);
        return element.token;
    }

    const char *name = "???";
    switch (expected.type) {
        case ETT_EOS:
            name = "eos";
            break;
        case ETT_INTEGER:
            name = "integer";
            break;
        case ETT_DECIMAL:
            name = "decimal";
            break;
        case ETT_STRING:
            name = "text";
            break;
        case ETT_IMPLICIT_WORD:
        case ETT_WORD:
            name = "word";
            break;
        case ETT_PREDEFINED_WORD:
            name = predefined2str(C, expected.predefined_word);
            break;
        case ETT_OPERATOR:
            name = mcapi_lex_operator2str(C->U, expected.op);
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
    C->predefined_word_mode = PWM_NORMAL;
}

void parser_change_mode(struct parse_controller *C, enum predefined_word_mode mode) {
    C->predefined_word_mode = mode;
}

// api

static void parser_userdata_constructor(morphine_instance_t I, void *data) {
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

static void parser_userdata_destructor(morphine_instance_t I, void *data) {
    struct mc_parser *P = data;
    mapi_allocator_free(I, P->stack.elements);
    parser_userdata_free_context(I, P->context.current);
    parser_userdata_free_context(I, P->context.trash);
}

MORPHINE_API struct mc_parser *mcapi_push_parser(morphine_coroutine_t U) {
    morphine_usertype_t usertype = {
        .name = MC_PARSER_USERDATA_TYPE,
        .size = sizeof(struct mc_parser),
        .constructor = parser_userdata_constructor,
        .destructor = parser_userdata_destructor,
        .compare = NULL,
        .hash = NULL,
        .metatable = false,
    };

    mapi_usertype_declare(U, usertype);

    struct mc_parser *P = mapi_push_userdata(U, MC_PARSER_USERDATA_TYPE);
    push_context(U, P, rule_root);

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
        .predefined_word_mode = PWM_NORMAL,
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
