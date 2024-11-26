//
// Created by why-iskra on 08.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_if(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    {
        parser_consume(C, et_predef_word(if));
        parser_consume(C, et_operator(LPAREN));
        parser_reduce(C, rule_expression);
        parser_consume(C, et_operator(RPAREN));
        parser_reduce(C, rule_statement_block);
        if (parser_match(C, et_predef_word(else))) {
            parser_reduce(C, rule_statement_block);
        }
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_expression_if *expression_if =
        mcapi_ast_create_expression_if(parser_U(C), parser_A(C), token_from, token_to, line);

    parser_consume(C, et_predef_word(if));
    parser_consume(C, et_operator(LPAREN));
    expression_if->condition = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
    parser_consume(C, et_operator(RPAREN));
    expression_if->if_statement = mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_block));
    if (parser_match(C, et_predef_word(else))) {
        expression_if->else_statement =
            mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_block));
    } else {
        expression_if->else_statement = NULL;
    }

    return mcapi_ast_expression_if2node(expression_if);
}
