//
// Created by why-iskra on 02.06.2024.
//

#include "morphinec/printer.h"

#define printn(str, s) do { mapi_sio_write(U, (const uint8_t *) (str), (s)); } while(false)
#define printf(args...) do { mapi_sio_printf(U, args); } while(false)

#define get_ast(s, t, n) struct s##_##t *n = ast_as_##s##_##t(U, ast_as_node(s))
#define get_ast_named(t, n) get_ast(t, n, n)

static void print_string(
    morphine_coroutine_t U,
    struct morphinec_strtable *T,
    morphinec_strtable_index_t index
) {
    struct morphinec_strtable_entry entry = mcapi_strtable_access(U, T, index);
    printn(entry.string, entry.size);
}

static void print_ast_statement(
    morphine_coroutine_t,
    struct morphinec_strtable *,
    size_t indent,
    struct statement *
);

static void print_ast_expression(
    morphine_coroutine_t U,
    struct morphinec_strtable *T,
    size_t indent,
    struct expression *expression
) {
    switch (expression->type) {
        case EXPRESSION_TYPE_value: {
            get_ast_named(expression, value);
            switch (value->type) {
                case EXPRESSION_VALUE_TYPE_NIL:
                    printf("nil");
                    break;
                case EXPRESSION_VALUE_TYPE_INT:
                    printf("%"MLIMIT_INTEGER_PR, value->value.integer);
                    break;
                case EXPRESSION_VALUE_TYPE_DEC:
                    printf("%"MLIMIT_DECIMAL_PR, value->value.decimal);
                    break;
                case EXPRESSION_VALUE_TYPE_STR:
                    printf("\"");
                    print_string(U, T, value->value.string);
                    printf("\"");
                    break;
                case EXPRESSION_VALUE_TYPE_BOOL:
                    if (value->value.boolean) {
                        printf("true");
                    } else {
                        printf("false");
                    }
                    break;
            }
            break;
        }
        case EXPRESSION_TYPE_binary: {
            get_ast_named(expression, binary);
            printf("(");
            print_ast_expression(U, T, indent, binary->a);
            switch (binary->type) {
                case EXPRESSION_BINARY_TYPE_ADD:
                    printf(" + ");
                    break;
                case EXPRESSION_BINARY_TYPE_SUB:
                    printf(" - ");
                    break;
                case EXPRESSION_BINARY_TYPE_MUL:
                    printf(" * ");
                    break;
                case EXPRESSION_BINARY_TYPE_DIV:
                    printf(" / ");
                    break;
                case EXPRESSION_BINARY_TYPE_MOD:
                    printf(" %% ");
                    break;
                case EXPRESSION_BINARY_TYPE_EQUAL:
                    printf(" == ");
                    break;
                case EXPRESSION_BINARY_TYPE_LESS:
                    printf(" < ");
                    break;
                case EXPRESSION_BINARY_TYPE_CONCAT:
                    printf(" .. ");
                    break;
                case EXPRESSION_BINARY_TYPE_AND:
                    printf(" and ");
                    break;
                case EXPRESSION_BINARY_TYPE_OR:
                    printf(" or ");
                    break;
            }
            print_ast_expression(U, T, indent, binary->b);
            printf(")");
            break;
        }
        case EXPRESSION_TYPE_unary: {
            get_ast_named(expression, unary);
            printf("(");
            switch (unary->type) {
                case EXPRESSION_UNARY_TYPE_NEGATE:
                    printf("- ");
                    break;
                case EXPRESSION_UNARY_TYPE_NOT:
                    printf("not ");
                    break;
                case EXPRESSION_UNARY_TYPE_TYPE:
                    printf("type ");
                    break;
                case EXPRESSION_UNARY_TYPE_LEN:
                    printf("len ");
                    break;
                case EXPRESSION_UNARY_TYPE_REF:
                    printf("ref ");
                    break;
                case EXPRESSION_UNARY_TYPE_DEREF:
                    printf("* ");
                    break;
            }
            print_ast_expression(U, T, indent, unary->expression);
            printf(")");
            break;
        }
        case EXPRESSION_TYPE_increment: {
            get_ast_named(expression, increment);
            char c = '+';
            if (increment->is_decrement) {
                c = '-';
            }

            printf("(");
            if (increment->is_postfix) {
                print_ast_expression(U, T, indent, increment->container);
                printf(" %c%c", c, c);
            } else {
                printf("%c%c ", c, c);
                print_ast_expression(U, T, indent, increment->container);
            }
            printf(")");
            break;
        }
        case EXPRESSION_TYPE_variable: {
            get_ast_named(expression, variable);
            print_string(U, T, variable->index);
            break;
        }
        case EXPRESSION_TYPE_global: {
            get_ast_named(expression, global);
            switch (global->type) {
                case EXPRESSION_GLOBAL_TYPE_ENV:
                    printf("env");
                    break;
                case EXPRESSION_GLOBAL_TYPE_SELF:
                    printf("self");
                    break;
            }
            break;
        }
        case EXPRESSION_TYPE_table: {
            get_ast_named(expression, table);
            printf("{ ");
            for (size_t i = 0; i < table->size; i++) {
                print_ast_expression(U, T, indent, table->keys[i]);
                printf(" to ");
                print_ast_expression(U, T, indent, table->values[i]);

                if (i == table->size - 1) {
                    printf(" ");
                } else {
                    printf(", ");
                }
            }
            printf("}");
            break;
        }
        case EXPRESSION_TYPE_vector: {
            get_ast_named(expression, vector);
            printf("[ ");
            for (size_t i = 0; i < vector->size; i++) {
                print_ast_expression(U, T, indent, vector->values[i]);

                if (i == vector->size - 1) {
                    printf(" ");
                } else {
                    printf(", ");
                }
            }
            printf("]");
            break;
        }
        case EXPRESSION_TYPE_access: {
            get_ast_named(expression, access);
            print_ast_expression(U, T, indent, access->container);
            printf("[");
            print_ast_expression(U, T, indent, access->key);
            printf("]");
            break;
        }
        case EXPRESSION_TYPE_call: {
            get_ast_named(expression, call);
            print_ast_expression(U, T, indent, call->expression);
            printf("(");
            for (size_t i = 0; i < call->args_size; i++) {
                print_ast_expression(U, T, indent, call->arguments[i]);

                if (i != call->args_size - 1) {
                    printf(", ");
                }
            }
            printf(")");
            break;
        }
        case EXPRESSION_TYPE_call_self: {
            get_ast_named(expression, call_self);
            print_ast_expression(U, T, indent, call_self->self);
            if (call_self->extract_callable) {
                printf(":[");
            } else {
                printf("->[");
            }
            print_ast_expression(U, T, indent, call_self->callable);
            printf("]");
            printf("(");
            for (size_t i = 0; i < call_self->args_size; i++) {
                print_ast_expression(U, T, indent, call_self->arguments[i]);

                if (i != call_self->args_size - 1) {
                    printf(", ");
                }
            }
            printf(")");
            break;
        }
        case EXPRESSION_TYPE_function: {
            get_ast_named(expression, function);
            printf("fun{%p}", function->ref);
            break;
        }
        case EXPRESSION_TYPE_block: {
            get_ast_named(expression, block);
            printf("eblk\n");
            for (size_t i = 0; i < block->size; i++) {
                for (size_t j = 0; j < indent + 1; j++) {
                    printf("    ");
                }

                print_ast_statement(U, T, indent + 1, block->statements[i]);
                printf("\n");
            }
            for (size_t j = 0; j < indent + 1; j++) {
                printf("    ");
            }
            print_ast_statement(U, T, indent + 1, ast_as_statement(block->result));
            printf("\n");
            for (size_t j = 0; j < indent; j++) {
                printf("    ");
            }
            printf("end");
            break;
        }
        case EXPRESSION_TYPE_if: {
            get_ast(expression, if, if_s);
            printf("if(");
            print_ast_expression(U, T, indent, if_s->if_condition);
            printf(") ");
            print_ast_expression(U, T, indent, if_s->if_expression);

            for (size_t i = 0; i < if_s->size; i++) {
                printf(" elif(");
                print_ast_expression(U, T, indent, if_s->elif_conditions[i]);
                printf(") ");
                print_ast_expression(U, T, indent, if_s->elif_expressions[i]);
            }

            printf(" else ");
            print_ast_expression(U, T, indent, if_s->else_expression);
            break;
        }
    }
}

static void print_ast_statement(
    morphine_coroutine_t U,
    struct morphinec_strtable *T,
    size_t indent,
    struct statement *statement
) {
    switch (statement->type) {
        case STATEMENT_TYPE_block: {
            get_ast_named(statement, block);
            printf("sblk\n");
            for (size_t i = 0; i < block->size; i++) {
                for (size_t j = 0; j < indent + 1; j++) {
                    printf("    ");
                }

                print_ast_statement(U, T, indent + 1, block->statements[i]);
                printf("\n");
            }
            for (size_t j = 0; j < indent; j++) {
                printf("    ");
            }
            printf("end");
            break;
        }
        case STATEMENT_TYPE_simple: {
            get_ast_named(statement, simple);
            switch (simple->type) {
                case STATEMENT_SIMPLE_TYPE_PASS:
                    printf("pass");
                    break;
                case STATEMENT_SIMPLE_TYPE_YIELD:
                    printf("yield");
                    break;
                case STATEMENT_SIMPLE_TYPE_LEAVE:
                    printf("leave");
                    break;
                case STATEMENT_SIMPLE_TYPE_BREAK:
                    printf("break");
                    break;
                case STATEMENT_SIMPLE_TYPE_CONTINUE:
                    printf("continue");
                    break;
            }
            break;
        }
        case STATEMENT_TYPE_eval: {
            get_ast_named(statement, eval);
            printf("eval ");
            if (eval->implicit) {
                printf("implicit ");
            } else {
                printf("explicit ");
            }
            print_ast_expression(U, T, indent, eval->expression);
            break;
        }
        case STATEMENT_TYPE_return: {
            get_ast(statement, return, ret);
            printf("return ");
            print_ast_expression(U, T, indent, ret->expression);
            break;
        }
        case STATEMENT_TYPE_while: {
            get_ast(statement, while, wh);
            if (wh->first_condition) {
                printf("while(");
                print_ast_expression(U, T, indent, wh->condition);
                printf(") ");
                print_ast_statement(U, T, indent, wh->statement);
            } else {
                printf("do ");
                print_ast_statement(U, T, indent, wh->statement);
                printf(" while(");
                print_ast_expression(U, T, indent, wh->condition);
                printf(")");
            }
            break;
        }
        case STATEMENT_TYPE_for: {
            get_ast(statement, for, fr);
            printf("for(");
            print_ast_statement(U, T, indent, fr->initial);
            printf("; ");
            print_ast_expression(U, T, indent, fr->condition);
            printf("; ");
            print_ast_statement(U, T, indent, fr->increment);
            printf(") ");
            print_ast_statement(U, T, indent, fr->statement);
            break;
        }
        case STATEMENT_TYPE_iterator: {
            get_ast_named(statement, iterator);
            printf("iterator(");
            if (iterator->size == 0) {
                print_string(U, T, iterator->name);
            } else {
                printf("decompose ");

                for (size_t i = 0; i < iterator->size; i++) {
                    print_string(U, T, iterator->multi.names[i]);

                    if (i != iterator->size - 1) {
                        printf(", ");
                    }
                }

                printf(" as ");

                for (size_t i = 0; i < iterator->size; i++) {
                    print_ast_expression(U, T, indent, iterator->multi.keys[i]);

                    if (i != iterator->size - 1) {
                        printf(", ");
                    }
                }
            }

            printf(" in ");
            print_ast_expression(U, T, indent, iterator->expression);
            printf(") ");
            print_ast_statement(U, T, indent, iterator->statement);
            break;
        }
        case STATEMENT_TYPE_declaration: {
            get_ast_named(statement, declaration);
            if (declaration->mutable) {
                printf("var ");
            } else {
                printf("val ");
            }

            if (declaration->size == 0) {
                print_string(U, T, declaration->name);
            } else {
                printf("decompose ");

                for (size_t i = 0; i < declaration->size; i++) {
                    print_string(U, T, declaration->multi.names[i]);

                    if (i != declaration->size - 1) {
                        printf(", ");
                    }
                }

                printf(" as ");

                for (size_t i = 0; i < declaration->size; i++) {
                    print_ast_expression(U, T, indent, declaration->multi.keys[i]);

                    if (i != declaration->size - 1) {
                        printf(", ");
                    }
                }
            }
            printf(" = ");
            print_ast_expression(U, T, indent, declaration->expression);
            break;
        }
        case STATEMENT_TYPE_assigment: {
            get_ast_named(statement, assigment);
            if (assigment->size == 0) {
                print_ast_expression(U, T, indent, assigment->container);
            } else {
                printf("decompose ");

                for (size_t i = 0; i < assigment->size; i++) {
                    print_ast_expression(U, T, indent, assigment->multi.containers[i]);

                    if (i != assigment->size - 1) {
                        printf(", ");
                    }
                }

                printf(" as ");

                for (size_t i = 0; i < assigment->size; i++) {
                    print_ast_expression(U, T, indent, assigment->multi.keys[i]);

                    if (i != assigment->size - 1) {
                        printf(", ");
                    }
                }
            }
            printf(" = ");
            print_ast_expression(U, T, indent, assigment->expression);
            break;
        }
        case STATEMENT_TYPE_if: {
            get_ast(statement, if, if_s);
            printf("if(");
            print_ast_expression(U, T, indent, if_s->if_condition);
            printf(") ");
            print_ast_statement(U, T, indent, if_s->if_statement);

            for (size_t i = 0; i < if_s->size; i++) {
                printf(" elif(");
                print_ast_expression(U, T, indent, if_s->elif_conditions[i]);
                printf(") ");
                print_ast_statement(U, T, indent, if_s->elif_statements[i]);
            }

            if (if_s->else_statement != NULL) {
                printf(" else ");
                print_ast_statement(U, T, indent, if_s->else_statement);
            }
            break;
        }
    }
}

static void print_ast_function(
    morphine_coroutine_t U,
    struct morphinec_strtable *T,
    struct ast_function *function
) {
    printf("    fun{%"MLIMIT_LINE_PR":%p}", function->line, function);
    if (!function->anonymous) {
        printf(" ");
        print_string(U, T, function->name);
    }

    if (function->auto_closure) {
        printf("<auto>(");
    } else if (function->closures_size > 0) {
        printf("<");
        for (size_t i = 0; i < function->closures_size; i++) {
            print_string(U, T, function->closures[i]);

            if (i != function->closures_size - 1) {
                printf(", ");
            }
        }
        printf(">(");
    } else {
        printf("(");
    }

    for (size_t i = 0; i < function->args_size; i++) {
        print_string(U, T, function->arguments[i]);

        if (i != function->args_size - 1) {
            printf(", ");
        }
    }
    printf(") ");

    if (function->statics_size > 0) {
        printf("static (");
        for (size_t i = 0; i < function->statics_size; i++) {
            print_string(U, T, function->statics[i]);

            if (i != function->statics_size - 1) {
                printf(", ");
            }
        }
        printf(") ");
    }

    print_ast_statement(U, T, 1, function->body);
    printf("\n");
}

void printer_strtable(morphine_coroutine_t U, struct morphinec_strtable *T) {
    printf("mcapi_push_strtable:\n");

    for (size_t i = 0; mcapi_strtable_has(T, i); i++) {
        struct morphinec_strtable_entry entry = mcapi_strtable_access(U, T, i);

        printf("  %zu. '", i);
        printn(entry.string, entry.size);
        printf("'\n");
    }

    printf("end\n\n");
}

void printer_ast(morphine_coroutine_t U, struct morphinec_strtable *T, struct ast *A) {
    struct ast_function *function = ast_functions(A);

    printf("ast:\n");
    while (function != NULL) {
        print_ast_function(U, T, function);
        function = function->prev;

        if (function != NULL) {
            printf("\n");
        }
    }

    printf("end\n\n");
}
