//
// Created by why-iskra on 13.08.2024.
//

#include "../controller.h"
#include "../extra/extract.h"

struct mc_ast_node *rule_iterator(struct parse_controller *C) {
    ml_size token_from = parser_index(C);
    ml_size decl_token_from;
    ml_size decl_token_to;
    size_t extract_size;
    {
        parser_consume(C, et_predef_word(iterator));
        parser_consume(C, et_operator(LPAREN));
        decl_token_from = parser_index(C);
        extract_size = extra_consume_extract(C, true, true);
        decl_token_to = parser_index(C);

        parser_consume(C, et_predef_word(in));
        if (parser_match(C, et_operator(LPAREN))) {
            parser_reduce(C, rule_expression);
            parser_consume(C, et_operator(COMMA));
            parser_reduce(C, rule_expression);
            parser_consume(C, et_operator(RPAREN));
        }

        parser_reduce(C, rule_expression);
        parser_consume(C, et_operator(RPAREN));
        parser_reduce(C, rule_statement_block);
    }
    ml_size token_to = parser_index(C);

    parser_reset(C);

    ml_line line = parser_get_line(C);

    struct mc_ast_statement_declaration *declaration = mcapi_ast_create_statement_declaration(
        parser_U(C), parser_A(C), decl_token_from, decl_token_to, line, extract_size
    );

    parser_consume(C, et_predef_word(iterator));
    parser_consume(C, et_operator(LPAREN));

    declaration->mutable = false;
    if (extract_size > 0) {
        extra_get_extract(
            C, true, true, declaration->extract.values, NULL, declaration->extract.keys
        );
    } else {
        extra_get_extract(
            C, true, true, &declaration->value, NULL, NULL
        );
    }

    struct mc_ast_expression *key = NULL;
    struct mc_ast_expression *value = NULL;
    parser_consume(C, et_predef_word(in));
    if (parser_match(C, et_operator(LPAREN))) {
        key = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
        parser_consume(C, et_operator(COMMA));
        value = mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
        parser_consume(C, et_operator(RPAREN));
    }

    declaration->expression =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

    parser_consume(C, et_operator(RPAREN));

    struct mc_ast_statement_iterator *iterator =
        mcapi_ast_create_statement_iterator(parser_U(C), parser_A(C), token_from, token_to, line);

    iterator->declaration = declaration;
    iterator->key = key;
    iterator->value = value;
    iterator->statement =
        mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_block));

    return mcapi_ast_statement_iterator2node(iterator);
}
