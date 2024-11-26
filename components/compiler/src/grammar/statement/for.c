//
// Created by why-iskra on 08.08.2024.
//

#include "../controller.h"

struct mc_ast_node *rule_for(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    {
        parser_consume(C, et_predef_word(for));
        parser_consume(C, et_operator(LPAREN));
        parser_reduce(C, rule_statement);
        parser_consume(C, et_operator(SEMICOLON));
        parser_reduce(C, rule_expression);
        parser_consume(C, et_operator(SEMICOLON));
        parser_reduce(C, rule_statement);
        parser_consume(C, et_operator(RPAREN));
        parser_reduce(C, rule_statement_block);
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    ml_line line = parser_get_line(C);

    parser_consume(C, et_predef_word(for));
    parser_consume(C, et_operator(LPAREN));
    struct mc_ast_statement *initial =
        mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement));
    parser_consume(C, et_operator(SEMICOLON));
    struct mc_ast_expression *condition =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
    parser_consume(C, et_operator(SEMICOLON));
    struct mc_ast_statement *increment =
        mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement));
    parser_consume(C, et_operator(RPAREN));
    struct mc_ast_statement *statement =
        mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_block));

    struct mc_ast_statement_for *statement_for =
        mcapi_ast_create_statement_for(parser_U(C), parser_A(C), token_from, token_to, line);

    statement_for->initial = initial;
    statement_for->condition = condition;
    statement_for->increment = increment;
    statement_for->statement = statement;

    return mcapi_ast_statement_for2node(statement_for);
}