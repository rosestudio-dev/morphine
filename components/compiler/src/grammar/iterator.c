//
// Created by why-iskra on 13.08.2024.
//

#include "controller.h"
#include "extra/extract.h"

struct mc_ast_node *rule_iterator(struct parse_controller *C) {
    size_t extract_size;
    {
        parser_consume(C, et_predef_word(iterator));
        parser_consume(C, et_operator(LPAREN));
        extract_size = extra_consume_extract(C, true);
        parser_consume(C, et_predef_word(in));
        parser_reduce(C, rule_expression);
        parser_consume(C, et_operator(RPAREN));
        parser_reduce(C, rule_statement_block);
    }

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_statement_declaration *declaration =
        mcapi_ast_create_statement_declaration(parser_U(C), parser_A(C), line, extract_size);

    parser_consume(C, et_predef_word(iterator));
    parser_consume(C, et_operator(LPAREN));

    declaration->mutable = false;
    if (extract_size > 0) {
        extra_get_extract(
            C, true, declaration->extract.values, NULL,
            declaration->extract.keys
        );
    } else {
        extra_get_extract(
            C, true, &declaration->value, NULL, NULL
        );
    }

    parser_consume(C, et_predef_word(in));

    declaration->expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

    parser_consume(C, et_operator(RPAREN));

    struct mc_ast_statement_iterator *iterator =
        mcapi_ast_create_statement_iterator(parser_U(C), parser_A(C), line);

    iterator->declaration = declaration;
    iterator->statement =
        mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_block));

    return mcapi_ast_statement_iterator2node(iterator);
}
