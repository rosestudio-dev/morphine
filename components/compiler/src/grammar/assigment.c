//
// Created by why-iskra on 01.06.2024.
//

#include "impl.h"
#include "support/decompose.h"

#define table_size (sizeof(assigment_table) / sizeof(assigment_table[0]))

static struct {
    enum token_operator operator;
    enum expression_binary_type binary_type;
} assigment_table[] = {
    { .operator = TOP_PLUSEQ, .binary_type = EXPRESSION_BINARY_TYPE_ADD },
    { .operator = TOP_MINUSEQ, .binary_type = EXPRESSION_BINARY_TYPE_SUB },
    { .operator = TOP_STAREQ, .binary_type = EXPRESSION_BINARY_TYPE_MUL },
    { .operator = TOP_SLASHEQ, .binary_type = EXPRESSION_BINARY_TYPE_DIV },
    { .operator = TOP_DOTDOTEQ, .binary_type = EXPRESSION_BINARY_TYPE_CONCAT },
};

static bool match_assigment_symbol(struct matcher *M) {
    if (matcher_match(M, symbol_operator(TOP_EQ))) {
        return true;
    }

    for (size_t i = 0; i < table_size; i++) {
        if (matcher_match(M, symbol_operator(assigment_table[i].operator))) {
            return true;
        }
    }

    return false;
}

void match_assigment(struct matcher *M) {
    if (match_decompose(M, false)) {
        matcher_consume(M, symbol_operator(TOP_EQ));
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    } else if (match_assigment_symbol(M)) {
        matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
    }
}

struct ast_node *assemble_assigment(morphine_coroutine_t U, struct elements *E) {
    size_t index = 0;
    size_t size = size_decompose(U, E, false, 0, &index);

    if (size == 0 && elements_size(E) == index) {
        struct ast_node *node = elements_get_reduce(E, 0).node;
        struct expression *expression = ast_node_as_expression(U, node);

        struct statement_eval *result = ast_create_statement_eval(U, elements_line(E, 0));

        enum expression_type type = ast_expression_type(expression);
        result->implicit = type != EXPRESSION_TYPE_call &&
                           type != EXPRESSION_TYPE_call_self &&
                           type != EXPRESSION_TYPE_increment;

        result->expression = expression;

        return ast_as_node(result);
    }

    struct statement_assigment *result = ast_create_statement_assigment(U, elements_line(E, 0), size);
    result->expression = ast_node_as_expression(U, elements_get_reduce(E, index + 1).node);

    if (size == 0) {
        insert_decompose(U, E, false, 0, NULL, &result->container, NULL);
    } else {
        insert_decompose(U, E, false, 0, NULL, result->multi.containers, result->multi.keys);
    }

    struct token eq = elements_get_token(E, index);
    if (size == 0 && !matcher_symbol(symbol_operator(TOP_EQ), eq)) {
        bool found = false;
        enum expression_binary_type type;
        for (size_t i = 0; i < table_size; i++) {
            if (matcher_symbol(symbol_operator(assigment_table[i].operator), eq)) {
                type = assigment_table[i].binary_type;
                found = true;
                break;
            }
        }

        if (!found) {
            return NULL;
        }

        struct expression_binary *binary = ast_create_expression_binary(U, elements_line(E, index));
        binary->type = type;
        binary->a = result->container;
        binary->b = result->expression;

        result->expression = ast_as_expression(binary);
    }

    return ast_as_node(result);
}