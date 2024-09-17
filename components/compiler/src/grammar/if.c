//
// Created by why-iskra on 08.08.2024.
//

#include "controller.h"
#include "extra/block.h"

static struct mc_ast_node *rule_statement_elif(struct parse_controller *C) {
    struct expected_token if_closes[] = {
        et_predef_word(elif),
        et_predef_word(else),
        et_predef_word(end)
    };

    struct expected_token else_closes[] = {
        et_predef_word(end)
    };

    size_t if_size;
    size_t else_size = 0;
    size_t close_index;
    {
        parser_consume(C, et_operator(LPAREN));
        parser_reduce(C, rule_expression);
        parser_consume(C, et_operator(RPAREN));

        if_size = extra_consume_statement_block(
            C, array_closes_size(if_closes), if_closes, &close_index
        );

        if (close_index == 0) {
            parser_reduce(C, rule_statement_elif);
        } else if (close_index == 1) {
            else_size = extra_consume_statement_block(
                C, array_closes_size(else_closes), else_closes, NULL
            );
        }
    }

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_statement_if *statement_if =
        mcapi_ast_create_statement_if(parser_U(C), parser_A(C), line);

    parser_consume(C, et_operator(LPAREN));
    statement_if->condition =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
    parser_consume(C, et_operator(RPAREN));

    ml_line if_line = parser_get_line(C);
    struct mc_ast_statement_block *if_block =
        mcapi_ast_create_statement_block(parser_U(C), parser_A(C), if_line, if_size);

    extra_extract_statement_block(
        C, array_closes_size(if_closes), if_closes, if_size, if_block->statements
    );

    statement_if->if_statement = mcapi_ast_block2statement(if_block);

    struct mc_ast_statement *else_statement = NULL;
    if (close_index == 0) {
        else_statement =
            mcapi_ast_node2statement(parser_U(C), parser_reduce(C, rule_statement_elif));
    } else if (close_index == 1) {
        ml_line else_line = parser_get_line(C);
        struct mc_ast_statement_block *else_block =
            mcapi_ast_create_statement_block(parser_U(C), parser_A(C), else_line, else_size);

        extra_extract_statement_block(
            C, array_closes_size(else_closes), else_closes, else_size, else_block->statements
        );

        else_statement = mcapi_ast_block2statement(else_block);
    }

    statement_if->else_statement = else_statement;

    return mcapi_ast_statement_if2node(statement_if);
}

static struct mc_ast_node *rule_expression_elif(struct parse_controller *C) {
    struct expected_token if_closes[] = {
        et_predef_word(elif),
        et_predef_word(else),
        et_predef_word(end)
    };

    struct expected_token else_closes[] = {
        et_predef_word(end)
    };

    size_t if_size;
    size_t else_size = 0;
    size_t close_index;
    {
        parser_consume(C, et_operator(LPAREN));
        parser_reduce(C, rule_expression);
        parser_consume(C, et_operator(RPAREN));

        if_size = extra_consume_expression_block(
            C, array_closes_size(if_closes), if_closes, &close_index
        );

        if (close_index == 0) {
            parser_reduce(C, rule_expression_elif);
        } else if (close_index == 1) {
            else_size = extra_consume_expression_block(
                C, array_closes_size(else_closes), else_closes, NULL
            );
        }
    }

    parser_reset(C);

    ml_line line = parser_get_line(C);
    struct mc_ast_expression_if *expression_if =
        mcapi_ast_create_expression_if(parser_U(C), parser_A(C), line);

    parser_consume(C, et_operator(LPAREN));
    expression_if->condition =
        mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
    parser_consume(C, et_operator(RPAREN));

    ml_line if_line = parser_get_line(C);
    struct mc_ast_expression_block *if_block =
        mcapi_ast_create_expression_block(parser_U(C), parser_A(C), if_line, if_size);

    extra_extract_expression_block(
        C, array_closes_size(if_closes), if_closes, if_size,
        if_block->statements, &if_block->expression
    );

    expression_if->if_expression = mcapi_ast_block2expression(if_block);

    struct mc_ast_expression *else_expression;
    if (close_index == 0) {
        else_expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression_elif));
    } else if (close_index == 1) {
        ml_line else_line = parser_get_line(C);
        struct mc_ast_expression_block *else_block =
            mcapi_ast_create_expression_block(parser_U(C), parser_A(C), else_line, else_size);

        extra_extract_expression_block(
            C, array_closes_size(else_closes), else_closes, else_size,
            else_block->statements, &else_block->expression
        );

        else_expression = mcapi_ast_block2expression(else_block);
    } else {
        parser_errorf(C, "expression if must contain else block");
    }

    expression_if->else_expression = else_expression;

    return mcapi_ast_expression_if2node(expression_if);
}

struct mc_ast_node *rule_statement_if(struct parse_controller *C) {
    parser_consume(C, et_predef_word(if));
    return parser_reduce(C, rule_statement_elif);
}

struct mc_ast_node *rule_expression_if(struct parse_controller *C) {
    parser_consume(C, et_predef_word(if));
    return parser_reduce(C, rule_expression_elif);
}
