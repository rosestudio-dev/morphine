//
// Created by why-iskra on 29.09.2024.
//

#include "controlflow.h"
#include "morphine/utils/overflow.h"

#define GROW_FACTOR 8

struct edge {
    ml_size source;
    ml_size target;
};

struct controlflow {
    size_t allocated;
    size_t size;
    struct edge *edges;
};

struct controlflow *controlflow_alloc(morphine_coroutine_t U) {
    struct controlflow *controlflow = mapi_allocator_uni(
        mapi_instance(U),
        NULL,
        sizeof(struct controlflow)
    );

    (*controlflow) = (struct controlflow) {
        .allocated = 0,
        .size = 0,
        .edges = NULL,
    };

    return controlflow;
}

void controlflow_free(morphine_instance_t I, struct controlflow *controlflow) {
    if (controlflow != NULL) {
        mapi_allocator_free(I, controlflow->edges);
        mapi_allocator_free(I, controlflow);
    }
}

void controlflow_add_edge(
    morphine_coroutine_t U,
    struct controlflow *controlflow,
    ml_size source,
    ml_size target
) {
    for (size_t i = 0; i < controlflow->size; i++) {
        struct edge edge = controlflow->edges[i];
        if (edge.source == source && edge.target == target) {
            return;
        }
    }

    if (controlflow->size == controlflow->allocated) {
        overflow_add(controlflow->allocated, GROW_FACTOR, SIZE_MAX) {
            mapi_error(U, "controlflow overflow");
        }

        controlflow->edges = mapi_allocator_vec(
            mapi_instance(U),
            controlflow->edges,
            controlflow->allocated + GROW_FACTOR,
            sizeof(struct edge)
        );

        controlflow->allocated += GROW_FACTOR;
    }

    controlflow->edges[controlflow->size] = (struct edge) {
        .source = source,
        .target = target
    };

    controlflow->size++;
}

static void fill(
    morphine_coroutine_t U,
    struct instructions *instructions,
    struct blocks *blocks,
    struct controlflow *controlflow
) {
    ml_size size = blocks_size(blocks);
    for (ml_size i = 0; i < size; i++) {
        struct blockedges outgoing = blocks_edges(U, instructions, blocks, i);

        for (size_t j = 0; j < outgoing.count; j++) {
            ml_size target = outgoing.edges[j];

            if (target < size) {
                controlflow_add_edge(U, controlflow, i, target);
            }
        }
    }
}

static void clear(morphine_coroutine_t U, struct controlflow *controlflow) {
    mapi_allocator_free(mapi_instance(U), controlflow->edges);

    (*controlflow) = (struct controlflow) {
        .allocated = 0,
        .size = 0,
        .edges = NULL,
    };
}

void controlflow_build(
    morphine_coroutine_t U,
    struct instructions *instructions,
    struct blocks *blocks,
    struct controlflow *controlflow
) {
    fill(U, instructions, blocks, controlflow);

    controlflow_dfs(U, controlflow, 0);
    blocks_reformat(U, blocks);

    clear(U, controlflow);
    fill(U, instructions, blocks, controlflow);
}

void controlflow_incoming_edges(morphine_coroutine_t U, struct controlflow *controlflow, ml_size vertex) {
    mapi_push_vector(U, 0);
    mapi_vector_mode_fixed(U, false);

    for (size_t i = 0; i < controlflow->size; i++) {
        struct edge edge = controlflow->edges[i];
        if (edge.target == vertex) {
            mapi_push_size(U, edge.source, "vertex");
            mapi_vector_push(U);
        }
    }
}

void controlflow_outgoing_edges(morphine_coroutine_t U, struct controlflow *controlflow, ml_size vertex) {
    mapi_push_vector(U, 0);
    mapi_vector_mode_fixed(U, false);

    for (size_t i = 0; i < controlflow->size; i++) {
        struct edge edge = controlflow->edges[i];
        if (edge.source == vertex) {
            mapi_push_size(U, edge.target, "vertex");
            mapi_vector_push(U);
        }
    }
}

static void vector_swap(morphine_coroutine_t U, ml_size from, ml_size to) {
    mapi_vector_get(U, from);
    mapi_rotate(U, 2);

    mapi_vector_get(U, to);
    mapi_vector_set(U, from);

    mapi_rotate(U, 2);
    mapi_vector_set(U, to);
}

void controlflow_dfs(morphine_coroutine_t U, struct controlflow *controlflow, ml_size vertex) {
    // visited
    mapi_push_vector(U, 0);
    mapi_vector_mode_fixed(U, false);

    // stack
    mapi_push_vector(U, 1);
    mapi_vector_mode_fixed(U, false);
    mapi_push_size(U, vertex, "vertex");
    mapi_vector_set(U, 0);

    while (mapi_vector_len(U) > 0) {
        mapi_vector_pop(U);
        ml_size current = mapi_get_size(U, "vertex");
        mapi_pop(U, 1);

        {
            bool has = false;

            mapi_rotate(U, 2);
            ml_size size = mapi_vector_len(U);
            for (ml_size i = 0; i < size; i++) {
                mapi_vector_get(U, i);
                ml_size visited = mapi_get_size(U, "vertex");
                mapi_pop(U, 1);

                if (visited == current) {
                    has = true;
                    break;
                }
            }

            if (has) {
                mapi_rotate(U, 2);
                continue;
            }

            mapi_push_size(U, current, "vertex");
            mapi_vector_push(U);
            mapi_rotate(U, 2);
        }

        ml_size found = 0;
        for (size_t i = 0; i < controlflow->size; i++) {
            struct edge edge = controlflow->edges[i];
            if (edge.source == current) {
                mapi_push_size(U, edge.target, "vertex");
                mapi_vector_push(U);
                found++;
            }
        }

        ml_size size = mapi_vector_len(U);
        for (ml_size i = 0; i < found / 2; i++) {
            vector_swap(U, size - i - 1, size - found + i);
        }
    }

    mapi_pop(U, 1);
}

#include <stdio.h>

void controlflow_dump(struct controlflow *controlflow) {
    printf("digraph G { ");
    for (size_t i = 0; i < controlflow->size; i++) {
        struct edge edge = controlflow->edges[i];
        printf("v%u -> v%u; ", edge.source, edge.target);
    }
    printf("}\n");
}
