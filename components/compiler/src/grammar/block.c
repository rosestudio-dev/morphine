//
// Created by why-iskra on 08.08.2024.
//

#include "controller.h"
#include "extra/block.h"

struct mc_ast_node *rule_statement_block(struct parse_controller *C) {
    struct expected_token closes[] = { et_predef_word(end) };
    size_t count = extra_consume_statement_block(C, array_closes_size(closes), closes, NULL);

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_statement_block *block =
        mcapi_ast_create_statement_block(parser_U(C), parser_A(C), line, count);

    extra_extract_statement_block(C, array_closes_size(closes), closes, count, block->statements);

    return mcapi_ast_statement_block2node(block);
}

struct mc_ast_node *rule_expression_block(struct parse_controller *C) {
    struct expected_token closes[] = { et_predef_word(end) };
    size_t count = extra_consume_expression_block(C, array_closes_size(closes), closes, NULL);

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_expression_block *block =
        mcapi_ast_create_expression_block(parser_U(C), parser_A(C), line, count);

    extra_extract_expression_block(
        C, array_closes_size(closes), closes, count, block->statements, &block->expression
    );

    return mcapi_ast_expression_block2node(block);
}
