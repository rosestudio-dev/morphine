//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"
#include "extra/block.h"

static void set_root(struct parse_controller *C, struct mc_ast_statement *body) {
    struct mc_ast_function *function =
        mcapi_ast_create_function(parser_U(C), parser_A(C), 0, 0, 0);

    function->line = 1;
    function->anonymous = true;
    function->recursive = false;
    function->auto_closure = false;
    function->body = body;

    mcapi_ast_set_root_function(parser_A(C), function);
}

struct mc_ast_node *rule_statement_root(struct parse_controller *C) {
    struct expected_token closes[] = { et_eos() };
    size_t count = extra_consume_statement_block(C, array_closes_size(closes), closes, NULL);

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_statement_block *block =
        mcapi_ast_create_statement_block(parser_U(C), parser_A(C), line, count);

    extra_extract_statement_block(C, array_closes_size(closes), closes, count, block->statements);

    set_root(C, mcapi_ast_block2statement(block));
    return mcapi_ast_statement_block2node(block);
}

struct mc_ast_node *rule_expression_root(struct parse_controller *C) {
    struct expected_token closes[] = { et_eos() };
    size_t count = extra_consume_expression_block(C, array_closes_size(closes), closes, NULL, true);

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_expression_block *block =
        mcapi_ast_create_expression_block(parser_U(C), parser_A(C), line, count);

    extra_extract_expression_block(
        C,
        array_closes_size(closes),
        closes,
        count,
        block->statements,
        &block->expression,
        true
    );

    struct mc_ast_expression_leave *leave =
        mcapi_ast_create_expression_leave(parser_U(C), parser_A(C), line);

    leave->expression = mcapi_ast_block2expression(block);

    struct mc_ast_statement_eval *eval =
        mcapi_ast_create_statement_eval(parser_U(C), parser_A(C), line);

    eval->implicit = false;
    eval->expression = mcapi_ast_leave2expression(leave);

    set_root(C, mcapi_ast_eval2statement(eval));
    return mcapi_ast_statement_eval2node(eval);
}
