//
// Created by why-iskra on 12.08.2024.
//

#include "controller.h"
#include "extra/extract.h"

#define assigment_table_size (sizeof(assigment_table) / sizeof(assigment_table[0]))

static struct {
    struct expected_token expected_token;
    enum mc_expression_binary_type binary_type;
} assigment_table[] = {
    { .expected_token = et_operator(PLUSEQ), .binary_type = MCEXPR_BINARY_TYPE_ADD },
    { .expected_token = et_operator(MINUSEQ), .binary_type = MCEXPR_BINARY_TYPE_SUB },
    { .expected_token = et_operator(STAREQ), .binary_type = MCEXPR_BINARY_TYPE_MUL },
    { .expected_token = et_operator(SLASHEQ), .binary_type = MCEXPR_BINARY_TYPE_DIV },
    { .expected_token = et_operator(PERCENTEQ), .binary_type = MCEXPR_BINARY_TYPE_MOD },
    { .expected_token = et_operator(DOTDOTEQ), .binary_type = MCEXPR_BINARY_TYPE_CONCAT },
};

static bool match_assigment_operator(struct parse_controller *C) {
    if (parser_match(C, et_operator(EQ))) {
        return true;
    }

    for (size_t i = 0; i < assigment_table_size; i++) {
        if (parser_match(C, assigment_table[i].expected_token)) {
            return true;
        }
    }

    return false;
}

static struct mc_ast_expression *build_assigment(
    struct parse_controller *C,
    struct mc_ast_expression *destination
) {
    for (size_t i = 0; i < assigment_table_size; i++) {
        ml_line line = parser_get_line(C);
        if (parser_match(C, assigment_table[i].expected_token)) {
            struct mc_ast_expression *expression =
                mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

            struct mc_ast_expression_binary *binary =
                mcapi_ast_create_expression_binary(parser_U(C), parser_A(C), line);

            binary->type = assigment_table[i].binary_type;
            binary->a = destination;
            binary->b = expression;

            return mcapi_ast_binary2expression(binary);
        }
    }

    parser_consume(C, et_operator(EQ));
    return mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
}

struct mc_ast_node *rule_assigment(struct parse_controller *C) {
    size_t extract_size;
    bool simple_expression = false;
    ml_line line;
    {
        extract_size = extra_consume_extract(C, false);
        line = parser_get_line(C);
        if (extract_size > 0) {
            parser_consume(C, et_operator(EQ));
            parser_reduce(C, rule_expression);
        } else if (match_assigment_operator(C)) {
            parser_reduce(C, rule_expression);
        } else {
            simple_expression = true;
        }
    }

    parser_reset(C);

    if (simple_expression) {
        struct mc_ast_expression *expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));

        struct mc_ast_statement_eval *eval =
            mcapi_ast_create_statement_eval(parser_U(C), parser_A(C), expression->node.line);

        eval->expression = expression;
        eval->implicit = expression->type != MCEXPRT_asm &&
                         expression->type != MCEXPRT_call &&
                         expression->type != MCEXPRT_leave &&
                         expression->type != MCEXPRT_break &&
                         expression->type != MCEXPRT_continue &&
                         expression->type != MCEXPRT_increment;

        return mcapi_ast_statement_eval2node(eval);
    }

    struct mc_ast_statement_assigment *assigment =
        mcapi_ast_create_statement_assigment(parser_U(C), parser_A(C), line, extract_size);

    if (extract_size > 0) {
        assigment->is_extract = true;
        extra_get_extract(
            C, false, NULL, assigment->extract.values, assigment->extract.keys
        );

        parser_consume(C, et_operator(EQ));

        assigment->expression =
            mcapi_ast_node2expression(parser_U(C), parser_reduce(C, rule_expression));
    } else {
        assigment->is_extract = false;
        extra_get_extract(
            C, false, NULL, &assigment->value, NULL
        );

        assigment->expression = build_assigment(C, assigment->value);
    }

    return mcapi_ast_statement_assigment2node(assigment);
}