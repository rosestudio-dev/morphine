//
// Created by why-iskra on 08.08.2024.
//

#include "controller.h"

static struct mc_ast_expression_block *block(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    size_t count = 0;
    {
        parser_consume(C, et_operator(LBRACE));
        while (!parser_match(C, et_operator(RBRACE))) {
            parser_reduce(C, rule_statement);
            parser_match(C, et_operator(SEMICOLON));
            count++;
        }
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_expression_block *block =
        mcapi_ast_create_expression_block(parser_U(C), parser_A(C), token_from, token_to, line, count);

    parser_consume(C, et_operator(LBRACE));

    for (size_t i = 0; i < count; i++) {
        block->statements[i] = mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement));
        parser_match(C, et_operator(SEMICOLON));
    }

    parser_consume(C, et_operator(RBRACE));

    return block;
}

struct mc_ast_node *rule_expression_block(struct parse_controller *C) {
    return mcapi_ast_expression_block2node(block(C));
}

struct mc_ast_node *rule_statement_block(struct parse_controller *C) {
    struct mc_ast_expression_block *result = block(C);
    struct mc_ast_statement_eval *eval = mcapi_ast_create_statement_eval(
        parser_U(C), parser_A(C), result->header.node.from, result->header.node.to, result->header.node.line
    );

    eval->expression = mcapi_ast_block2expression(result);
    return mcapi_ast_statement_eval2node(eval);
}
