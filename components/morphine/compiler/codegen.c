//
// Created by why-iskra on 02.06.2024.
//

#include <string.h>
#include "morphine/compiler/codegen.h"
#include "morphine/compiler/ast.h"

#define MORPHINE_TYPE "codegen"

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
    struct ast_node *root = ast_root(U);

    struct codegen *C = mapi_push_userdata(U, MORPHINE_TYPE, sizeof(struct codegen));

    *C = (struct codegen) {
        .root = root
    };

    mapi_userdata_set_free(U, codegen_free);
}
