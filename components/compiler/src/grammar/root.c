//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"

static void set_root(struct parse_controller *C, struct mc_ast_statement *body) {
    struct mc_ast_function *function =
        mcapi_ast_create_function(parser_U(C), parser_A(C), 0, 0);

    function->line = 1;
    function->anonymous = true;
    function->recursive = false;
    function->auto_closure = false;
    function->body = body;

    mcapi_ast_set_root_function(parser_A(C), function);
}

struct mc_ast_node *rule_root(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    size_t count = 0;
    {
        while (!parser_match(C, et_eos())) {
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

    for (size_t i = 0; i < count; i++) {
        block->statements[i] = mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement));
        parser_match(C, et_operator(SEMICOLON));
    }

    parser_consume(C, et_eos());

    struct mc_ast_statement_eval *eval =
        mcapi_ast_create_statement_eval(parser_U(C), parser_A(C), token_from, token_to, line);
    eval->expression = mcapi_ast_block2expression(block);

    set_root(C, mcapi_ast_eval2statement(eval));
    return mcapi_ast_statement_eval2node(eval);
}
