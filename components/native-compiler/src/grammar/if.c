//
// Created by why-iskra on 01.06.2024.
//

#include "impl.h"
#include "support/block.h"

#define table_size(t) (sizeof(t) / sizeof((t)[0]))

void match_statement_if(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(TPW_if));
    matcher_consume(M, symbol_operator(TOP_LPAREN));
    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    matcher_consume(M, symbol_operator(TOP_RPAREN));

    struct matcher_symbol if_closes[] = {
        symbol_predef_word(TPW_elif),
        symbol_predef_word(TPW_else),
        symbol_predef_word(TPW_end)
    };

    size_t if_closed = match_sblock(M, table_size(if_closes), if_closes);

    if (if_closed == 1) {
        goto match_else;
    } else if (if_closed == 2) {
        return;
    }

    while (true) {
        matcher_consume(M, symbol_operator(TOP_LPAREN));
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        matcher_consume(M, symbol_operator(TOP_RPAREN));
        size_t elif_closed = match_sblock(M, table_size(if_closes), if_closes);

        if (elif_closed == 1) {
            goto match_else;
        }

        if (elif_closed == 2) {
            return;
        }
    }

match_else:;
    struct matcher_symbol else_closes[] = {
        symbol_predef_word(TPW_end)
    };

    match_sblock(M, table_size(else_closes), else_closes);
}

void match_expression_if(struct matcher *M) {
    matcher_consume(M, symbol_predef_word(TPW_if));
    matcher_consume(M, symbol_operator(TOP_LPAREN));
    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    matcher_consume(M, symbol_operator(TOP_RPAREN));

    struct matcher_symbol if_closes[] = {
        symbol_predef_word(TPW_elif),
        symbol_predef_word(TPW_else)
    };

    size_t if_closed = match_eblock(M, table_size(if_closes), if_closes);

    if (if_closed == 1) {
        goto match_else;
    }

    while (true) {
        matcher_consume(M, symbol_operator(TOP_LPAREN));
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
        matcher_consume(M, symbol_operator(TOP_RPAREN));
        size_t elif_closed = match_eblock(M, table_size(if_closes), if_closes);

        if (elif_closed == 1) {
            goto match_else;
        }
    }

match_else:;
    struct matcher_symbol else_closes[] = {
        symbol_predef_word(TPW_end)
    };

    match_eblock(M, table_size(else_closes), else_closes);
}

static size_t count_elif(struct elements *E) {
    size_t count = 0;
    size_t size = elements_size(E);
    for (size_t i = 0; i < size; i++) {
        if (elements_look(E, i, symbol_predef_word(TPW_elif))) {
            count++;
        }
    }

    return count;
}

struct ast_node *assemble_statement_if(morphine_coroutine_t U, struct elements *E) {
    size_t elifs = count_elif(E);

    struct statement_if *result = ast_create_statement_if(U, elements_line(E, 0), elifs);
    result->condition = ast_node_as_expression(U, elements_get_reduce(E, 2).node);
    result->else_statement = NULL;

    struct matcher_symbol if_closes[] = {
        symbol_predef_word(TPW_elif),
        symbol_predef_word(TPW_else),
        symbol_predef_word(TPW_end)
    };

    size_t end_index = 0;

    size_t if_closed = get_sblock(
        U, E, table_size(if_closes), if_closes, 4,
        &result->if_statement, &end_index
    );

    if (if_closed == 1) {
        goto match_else;
    } else if (if_closed == 2) {
        return ast_as_node(result);
    }

    size_t count = 0;
    while (true) {
        result->elif_conditions[count] = ast_node_as_expression(
            U, elements_get_reduce(E, end_index + 2).node
        );

        size_t elif_closed = get_sblock(
            U, E, table_size(if_closes), if_closes, end_index + 4,
            result->elif_statements + count, &end_index
        );

        if (elif_closed == 1) {
            goto match_else;
        }

        if (elif_closed == 2) {
            return ast_as_node(result);
        }

        count++;
    }

match_else:;
    struct matcher_symbol else_closes[] = {
        symbol_predef_word(TPW_end)
    };

    get_sblock(
        U, E, table_size(else_closes), else_closes, end_index + 1,
        &result->else_statement, NULL
    );

    return ast_as_node(result);
}

struct ast_node *assemble_expression_if(morphine_coroutine_t U, struct elements *E) {
    size_t elifs = count_elif(E);

    struct expression_if *result = ast_create_expression_if(U, elements_line(E, 0), elifs);
    result->condition = ast_node_as_expression(U, elements_get_reduce(E, 2).node);
    result->else_expression = NULL;

    struct matcher_symbol if_closes[] = {
        symbol_predef_word(TPW_elif),
        symbol_predef_word(TPW_else)
    };

    size_t end_index = 0;

    size_t if_closed = get_eblock(
        U, E, table_size(if_closes), if_closes, 4,
        &result->if_expression, &end_index
    );

    if (if_closed == 1) {
        goto match_else;
    }

    size_t count = 0;
    while (true) {
        result->elif_conditions[count] = ast_node_as_expression(
            U, elements_get_reduce(E, end_index + 2).node
        );

        size_t elif_closed = get_eblock(
            U, E, table_size(if_closes), if_closes, end_index + 4,
            result->elif_expressions + count, &end_index
        );

        if (elif_closed == 1) {
            goto match_else;
        }

        count++;
    }

match_else:;
    struct matcher_symbol else_closes[] = {
        symbol_predef_word(TPW_end)
    };

    get_eblock(
        U, E, table_size(else_closes), else_closes, end_index + 1,
        &result->else_expression, NULL
    );

    return ast_as_node(result);
}
