//
// Created by why-iskra on 01.06.2024.
//

#include "impl.h"
#include "support/decompose.h"

void match_declaration(struct matcher *M) {
    if (matcher_look(M, symbol_predef_word(TPW_fun)) || matcher_is_reduced(M, REDUCE_TYPE_FUNCTION)) {
        matcher_reduce(M, REDUCE_TYPE_FUNCTION);
        return;
    }

    if (!matcher_match(M, symbol_predef_word(TPW_val)) && !matcher_match(M, symbol_predef_word(TPW_var))) {
        matcher_error(M, "expect val, var or fun");
    }

    match_decompose(M, true);

    matcher_consume(M, symbol_operator(TOP_EQ));
    matcher_reduce(M, REDUCE_TYPE_EXPRESSION);
}

struct ast_node *assemble_declaration(morphine_coroutine_t U, struct elements *E) {
    if (!elements_is_token(E, 0)) {
        struct reduce reduce = elements_get_reduce(E, 0);
        struct expression_function *function = ast_as_expression_function(U, reduce.node);
        if (function->ref->anonymous) {
            elements_error(E, 0, "cannot declare anonymous function");
        }

        struct statement_declaration *result = ast_create_statement_declaration(U, reduce.node->line, 0);
        result->mutable = false;
        result->expression = ast_as_expression(function);
        result->name = function->ref->name;

        return ast_as_node(result);
    }

    size_t index = 0;
    size_t size = size_decompose(U, E, true, 1, &index);

    struct statement_declaration *result = ast_create_statement_declaration(U, elements_line(E, 0), size);
    result->mutable = elements_look(E, 0, symbol_predef_word(TPW_var));
    result->expression = ast_node_as_expression(U, elements_get_reduce(E, index + 1).node);

    if (size == 0) {
        insert_decompose(U, E, true, 1, &result->name, NULL, NULL);
    } else {
        insert_decompose(U, E, true, 1, result->multi.names, NULL, result->multi.keys);
    }

    return ast_as_node(result);
}