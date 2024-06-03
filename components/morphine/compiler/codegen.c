//
// Created by why-iskra on 02.06.2024.
//

#include <string.h>
#include <stdio.h>
#include "morphine/compiler/codegen.h"
#include "morphine/compiler/ast.h"
#include "morphine/compiler/visitor.h"

#define MORPHINE_TYPE "codegen"

enum codegen_argument_type {
    CAT_VARIABLE,
    CAT_TEMPORARY,
    CAT_ANCHOR,
    CAT_FUNCTION,
    CAT_STABLE,
};

struct codegen_argument_action {
    enum codegen_argument_type type;
    union {
        size_t variable;
        size_t temporary;
        size_t anchor;
        intptr_t function;
        ml_argument stable;
    };
};

struct codegen_instruction {
    morphine_instruction_t instruction;
    struct codegen_argument_action argument1_action;
    struct codegen_argument_action argument2_action;
    struct codegen_argument_action argument3_action;
};

struct codegen {
};

//static void codegen_free(morphine_instance_t I, void *p) {
//    struct codegen *C = p;
//    (void) C;
//    (void) I;
//}

//static struct codegen *get_codegen(morphine_coroutine_t U) {
//    mapi_peek(U, 1);
//    if (strcmp(mapi_userdata_type(U), MORPHINE_TYPE) == 0) {
//        struct codegen *C = mapi_userdata_pointer(U);
//        mapi_pop(U, 1);
//        return C;
//    } else {
//        mapi_error(U, "expected "MORPHINE_TYPE);
//    }
//}

static void visit_statement(struct visitor_controller *C, struct statement *statement, size_t state) {
    (void) C;
    (void) state;
    switch (ast_statement_type(statement)) {
        case STATEMENT_TYPE_block:
            printf("block");
            break;
        case STATEMENT_TYPE_simple:
            printf("simple");
            break;
        case STATEMENT_TYPE_eval:
            printf("eval");
            break;
        case STATEMENT_TYPE_return:
            printf("return");
            break;
        case STATEMENT_TYPE_while:
            printf("while");
            break;
        case STATEMENT_TYPE_for:
            printf("for");
            break;
        case STATEMENT_TYPE_iterator:
            printf("iterator");
            break;
        case STATEMENT_TYPE_declaration:
            printf("declaration");
            break;
        case STATEMENT_TYPE_assigment:
            printf("assigment");
            break;
        case STATEMENT_TYPE_if:
            printf("if");
            break;
    }
    printf("\n");
}

static void visit(struct visitor_controller *C, struct ast_node *node, size_t state) {
    if (ast_node_type(node) == AST_NODE_TYPE_EXPRESSION) {
        visitor_return(C);
    } else {
        visit_statement(C, ast_as_statement(node), state);
        visitor_return(C);
    }
}

void codegen(morphine_coroutine_t U) {
    visitor(U, visit, NULL);
    while (visitor_step(U, NULL)) { }

//    struct codegen *C = mapi_push_userdata(U, MORPHINE_TYPE, sizeof(struct codegen));
//
//    *C = (struct codegen) {
////        .root = root
//    };
//
//    mapi_userdata_set_free(U, codegen_free);
}
