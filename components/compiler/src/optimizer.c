//
// Created by why-iskra on 29.09.2024.
//

#include <stdio.h>
#include "morphinec/optimizer.h"
#include "optimizer/domtree.h"

struct optimizer {
    struct instructions *instructions;
    struct blocks *blocks;
    struct controlflow *controlflow;
    struct domtree *domtree;
};

static void free_optimizer(morphine_instance_t I, void *data) {
    struct optimizer *P = data;
    instructions_free(I, P->instructions);
    blocks_free(I, P->blocks);
    controlflow_free(I, P->controlflow);
    domtree_free(I, P->domtree);
}

static struct optimizer *create_optimizer(morphine_coroutine_t U) {
    struct optimizer *P = mapi_push_userdata_uni(U, sizeof(struct optimizer));
    (*P) = (struct optimizer) {
        .instructions = NULL,
        .blocks = NULL,
        .controlflow = NULL,
        .domtree = NULL,
    };

    mapi_userdata_set_free(U, free_optimizer);

    return P;
}

MORPHINE_API bool mcapi_optimize(morphine_coroutine_t U) {
    struct optimizer *P = create_optimizer(U);
    mapi_rotate(U, 2);

    P->instructions = instructions_alloc(U);
    P->blocks = blocks_alloc(U);
    P->controlflow = controlflow_alloc(U);
    P->domtree = domtree_alloc(U);

    instructions_build(U, P->instructions);
    blocks_build(U, P->instructions, P->blocks);
    controlflow_build(U, P->instructions, P->blocks, P->controlflow);
    domtree_build(U, P->blocks, P->controlflow, P->domtree);
    controlflow_dump(P->controlflow);
    domtree_dump(P->domtree);

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
    return false;
}
