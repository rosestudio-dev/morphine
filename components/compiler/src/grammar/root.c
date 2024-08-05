//
// Created by why-iskra on 05.08.2024.
//

#include "controller.h"
#include "extra/block.h"

struct mc_ast_node *parse_root(struct parse_controller *C) {
    struct mc_lex_token closes[] = { et_eos() };
    size_t count = extra_consume_statement_block(C, array_closes_size(closes), closes, NULL);

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_statement_block *block =
        mcapi_ast_create_statement_block(parser_U(C), parser_A(C), line, count);

    extra_extract_statement_block(C, array_closes_size(closes), closes, count, block->statements);

    struct mc_ast_function *function =
        mcapi_ast_create_function(parser_U(C), parser_A(C), 0, 0, 0);

    function->line = 1;
    function->anonymous = true;
    function->recursive = false;
    function->auto_closure = false;
    function->body = mcapi_ast_block2statement(block);

    mcapi_ast_set_root_function(parser_A(C), function);

    return mcapi_ast_statement_block2node(block);
}
