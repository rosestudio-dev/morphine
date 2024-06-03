//
// Created by why-iskra on 02.06.2024.
//

#include <string.h>
#include "morphine/compiler/codegen.h"
#include "morphine/compiler/ast.h"

#define MORPHINE_TYPE "codegen"

enum codegen_argument_type {
    CAT_VARIABLE,
    CAT_TEMPORARY,
    CAT_ANCHOR,
    CAT_STABLE,
};

struct codegen_argument_action {
    enum codegen_argument_type type;
    union {
        size_t variable;
        size_t temporary;
        size_t anchor;
        ml_argument stable;
    };
};

struct codegen_instruction {
    morphine_instruction_t instruction;
    struct codegen_argument_action argument1_action;
    struct codegen_argument_action argument2_action;
    struct codegen_argument_action argument3_action;
};

struct codegen_code_region {
    bool is_compiled;
    union {
        struct {
            size_t size;
            struct codegen_instruction *instructions;
        } compiled;

        struct {

        } precompiled;
    };
};

struct codegen_function {

};

struct codegen {
    struct ast_node *root;
};

static void codegen_free(morphine_instance_t I, void *p) {
    struct codegen *C = p;
    (void) C;
    (void) I;
}

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

void codegen(morphine_coroutine_t U) {
//    struct ast_node *root = ast_root(U);

    struct codegen *C = mapi_push_userdata(U, MORPHINE_TYPE, sizeof(struct codegen));

    *C = (struct codegen) {
//        .root = root
    };

    mapi_userdata_set_free(U, codegen_free);
}
