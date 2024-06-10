//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"
#include "support/arguments.h"

void match_function(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(TPW_fun));

    if (matcher_match(M, symbol_predef_word(TPW_recursive))) {
        matcher_consume(M, symbol_word);
    } else {
        matcher_match(M, symbol_word);
    }

    struct argument_matcher R = {
        .assemble = false,
        .M = M,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
    };

    if (matcher_look(M, symbol_operator(TOP_LT))) {
        R.open_symbol = symbol_operator(TOP_LT);
        R.close_symbol = symbol_operator(TOP_GT);

        if (argument_matcher_init(&R, 0)) {
            if (!matcher_match(M, symbol_predef_word(TPW_auto))) {
                do {
                    matcher_consume(M, symbol_word);
                } while (argument_matcher_next(&R));
            }
        }
        argument_matcher_close(&R);
    }

    R.open_symbol = symbol_operator(TOP_LPAREN);
    R.close_symbol = symbol_operator(TOP_RPAREN);

    if (argument_matcher_init(&R, 0)) {
        do {
            matcher_consume(M, symbol_word);
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);

    if (matcher_match(M, symbol_predef_word(TPW_static))) {
        if (argument_matcher_init(&R, 0)) {
            do {
                matcher_consume(M, symbol_word);
            } while (argument_matcher_next(&R));
        }
        argument_matcher_close(&R);
    }

    if (matcher_match(M, symbol_operator(TOP_EQ))) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    } else {
        matcher_reduce(M, REDUCE_TYPE_STATEMENT_BLOCK);
    }
}

static struct ast_function *create_function(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    size_t start_pos;
    bool recursive;
    bool anonymous;
    strtable_index_t name;
    if (elements_look(E, 1, symbol_predef_word(TPW_recursive))) {
        recursive = true;
        anonymous = false;
        name = elements_get_token(E, 2).word;
        start_pos = 3;
    } else if (elements_look(E, 1, symbol_word)) {
        recursive = false;
        anonymous = false;
        name = elements_get_token(E, 1).word;
        start_pos = 2;
    } else {
        recursive = false;
        anonymous = true;
        name = 0;
        start_pos = 1;
    }

    struct argument_matcher R = {
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
        R.open_symbol = symbol_operator(TOP_LT);
        R.close_symbol = symbol_operator(TOP_GT);

        if (argument_matcher_init(&R, start_pos)) {
            if (argument_matcher_match(&R, symbol_predef_word(TPW_auto))) {
                auto_closure = true;
            } else {
                do {
                    argument_matcher_consume(&R, symbol_word);
                } while (argument_matcher_next(&R));
            }
        }
        closures = argument_matcher_close(&R);
    }

    R.open_symbol = symbol_operator(TOP_LPAREN);
    R.close_symbol = symbol_operator(TOP_RPAREN);

    if (argument_matcher_init(&R, R.pos)) {
        do {
            argument_matcher_consume(&R, symbol_word);
        } while (argument_matcher_next(&R));
    }
    size_t args = argument_matcher_close(&R);

    size_t statics = 0;
    if (elements_look(E, R.pos, symbol_predef_word(TPW_static))) {
        if (argument_matcher_init(&R, R.pos + 1)) {
            do {
                argument_matcher_consume(&R, symbol_word);
            } while (argument_matcher_next(&R));
        }
        statics = argument_matcher_close(&R);
    }

    struct ast_node *body;
    if (elements_look(E, R.pos, symbol_operator(TOP_EQ))) {
        struct reduce reduce = elements_get_reduce(E, R.pos + 1);
        ml_line line = elements_get_token(E, R.pos).line;

        struct statement_return *ret = ast_create_statement_return(U, A, line);
        ret->expression = ast_node_as_expression(U, reduce.node);

        body = ast_as_node(ret);
    } else {
        struct reduce reduce = elements_get_reduce(E, R.pos);
        body = reduce.node;
    }

    struct ast_function *function = ast_create_function(U, A, closures, args, statics);
    function->line = elements_get_token(E, 0).line;
    function->recursive = recursive;
    function->anonymous = anonymous;
    function->name = name;
    function->auto_closure = auto_closure;
    function->body = ast_node_as_statement(U, body);

    return function;
}

struct ast_node *assemble_function(morphine_coroutine_t U, struct ast *A, struct elements *E) {
    struct ast_function *function = create_function(U, A, E);

    size_t start_pos;
    if (elements_look(E, 1, symbol_predef_word(TPW_recursive))) {
        start_pos = 3;
    } else if (elements_look(E, 1, symbol_word)) {
        start_pos = 2;
    } else {
        start_pos = 1;
    }

    struct argument_matcher R = {
        .assemble = true,
        .E = E,
        .U = U,
        .separator = symbol_operator(TOP_COMMA),
        .has_open_close = true,
        .consume_open = true,
        .pos = start_pos,
    };

    if (elements_look(E, start_pos, symbol_operator(TOP_LT))) {
        R.open_symbol = symbol_operator(TOP_LT);
        R.close_symbol = symbol_operator(TOP_GT);

        if (argument_matcher_init(&R, start_pos)) {
            if (!argument_matcher_match(&R, symbol_predef_word(TPW_auto))) {
                do {
                    struct token token = argument_matcher_consume(&R, symbol_word);
                    function->closures[R.count] = token.word;
                } while (argument_matcher_next(&R));
            }
        }
        argument_matcher_close(&R);
    }

    R.open_symbol = symbol_operator(TOP_LPAREN);
    R.close_symbol = symbol_operator(TOP_RPAREN);

    if (argument_matcher_init(&R, R.pos)) {
        do {
            struct token token = argument_matcher_consume(&R, symbol_word);
            function->arguments[R.count] = token.word;
        } while (argument_matcher_next(&R));
    }
    argument_matcher_close(&R);

    if (elements_look(E, R.pos, symbol_predef_word(TPW_static))) {
        if (argument_matcher_init(&R, R.pos + 1)) {
            do {
                struct token token = argument_matcher_consume(&R, symbol_word);
                function->statics[R.count] = token.word;
            } while (argument_matcher_next(&R));
        }
        argument_matcher_close(&R);
    }

    struct expression_function *result = ast_create_expression_function(U, A, function->line);
    result->ref = function;

    return ast_as_node(result);
}