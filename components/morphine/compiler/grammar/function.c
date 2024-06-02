//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"
#include "support/arguments.h"

void match_function(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(TPW_fun));
    matcher_match(M, symbol_word);

    struct argument_matcher A = {
        .assemble = false,
        .M = M,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
    };

    if (matcher_look(M, symbol_operator(TOP_LT))) {
        A.open_symbol = symbol_operator(TOP_LT);
        A.close_symbol = symbol_operator(TOP_GT);

        if (argument_matcher_init(&A, 0)) {
            if (!matcher_match(M, symbol_predef_word(TPW_auto))) {
                do {
                    matcher_consume(M, symbol_word);
                } while (argument_matcher_next(&A));
            }
        }
        argument_matcher_close(&A);
    }

    A.open_symbol = symbol_operator(TOP_LPAREN);
    A.close_symbol = symbol_operator(TOP_RPAREN);

    if (argument_matcher_init(&A, 0)) {
        do {
            matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        } while (argument_matcher_next(&A));
    }
    argument_matcher_close(&A);

    if (matcher_match(M, symbol_predef_word(TPW_static))) {
        if (argument_matcher_init(&A, 0)) {
            do {
                matcher_consume(M, symbol_word);
            } while (argument_matcher_next(&A));
        }
        argument_matcher_close(&A);
    }

    if (matcher_match(M, symbol_operator(TOP_EQ))) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    } else {
        matcher_reduce(M, REDUCE_TYPE_STATEMENT_BLOCK);
    }
}

static struct expression_function *create_function(morphine_coroutine_t U, struct elements *E) {
    size_t start_pos;
    bool anonymous;
    strtable_index_t name;
    if (elements_look(E, 1, symbol_word)) {
        anonymous = false;
        name = elements_get_token(E, 1).word;
        start_pos = 2;
    } else {
        anonymous = true;
        name = 0;
        start_pos = 1;
    }

    struct argument_matcher A = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .pos = start_pos,
    };

    bool auto_closure = false;
    size_t closures = 0;
    if (elements_look(E, start_pos, symbol_operator(TOP_LT))) {
        A.open_symbol = symbol_operator(TOP_LT);
        A.close_symbol = symbol_operator(TOP_GT);

        if (argument_matcher_init(&A, start_pos)) {
            if (argument_matcher_match(&A, symbol_predef_word(TPW_auto))) {
                auto_closure = true;
            } else {
                do {
                    argument_matcher_consume(&A, symbol_word);
                } while (argument_matcher_next(&A));
            }
        }
        closures = argument_matcher_close(&A);
    }

    A.open_symbol = symbol_operator(TOP_LPAREN);
    A.close_symbol = symbol_operator(TOP_RPAREN);

    if (argument_matcher_init(&A, A.pos)) {
        do {
            argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);
        } while (argument_matcher_next(&A));
    }
    size_t args = argument_matcher_close(&A);

    size_t statics = 0;
    if (elements_look(E, A.pos, symbol_predef_word(TPW_static))) {
        if (argument_matcher_init(&A, A.pos + 1)) {
            do {
                argument_matcher_consume(&A, symbol_word);
            } while (argument_matcher_next(&A));
        }
        statics = argument_matcher_close(&A);
    }

    struct ast_node *body;
    if (elements_look(E, A.pos, symbol_operator(TOP_EQ))) {
        struct reduce reduce = elements_get_reduce(E, A.pos + 1);
        ml_line line = elements_get_token(E, A.pos).line;

        struct statement_return *ret = ast_create_statement_return(U, line);
        ret->expression = ast_node_as_expression(U, reduce.node);

        body = ast_as_node(ret);
    } else {
        struct reduce reduce = elements_get_reduce(E, A.pos);
        body = reduce.node;
    }

    ml_line line = elements_get_token(E, 0).line;
    struct expression_function *function = ast_create_expression_function(U, line, closures, args, statics);
    function->anonymous = anonymous;
    function->name = name;
    function->auto_closure = auto_closure;
    function->body = ast_node_as_statement(U, body);

    return function;
}

struct ast_node *assemble_function(morphine_coroutine_t U, struct elements *E) {
    struct expression_function *function = create_function(U, E);

    size_t start_pos;
    if (elements_look(E, 1, symbol_word)) {
        start_pos = 2;
    } else {
        start_pos = 1;
    }

    struct argument_matcher A = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .pos = start_pos,
    };

    if (elements_look(E, start_pos, symbol_operator(TOP_LT))) {
        A.open_symbol = symbol_operator(TOP_LT);
        A.close_symbol = symbol_operator(TOP_GT);

        if (argument_matcher_init(&A, start_pos)) {
            if (!argument_matcher_match(&A, symbol_predef_word(TPW_auto))) {
                do {
                    struct token token = argument_matcher_consume(&A, symbol_word);
                    function->closures[A.count] = token.word;
                } while (argument_matcher_next(&A));
            }
        }
        argument_matcher_close(&A);
    }

    A.open_symbol = symbol_operator(TOP_LPAREN);
    A.close_symbol = symbol_operator(TOP_RPAREN);

    if (argument_matcher_init(&A, A.pos)) {
        do {
            struct reduce reduce = argument_matcher_reduce(&A, REDUCE_TYPE_EXPRESSION);
            function->arguments[A.count] = ast_node_as_expression(U, reduce.node);
        } while (argument_matcher_next(&A));
    }
    argument_matcher_close(&A);

    if (elements_look(E, A.pos, symbol_predef_word(TPW_static))) {
        if (argument_matcher_init(&A, A.pos + 1)) {
            do {
                struct token token = argument_matcher_consume(&A, symbol_word);
                function->statics[A.count] = token.word;
            } while (argument_matcher_next(&A));
        }
        argument_matcher_close(&A);
    }

    return ast_as_node(function);
}