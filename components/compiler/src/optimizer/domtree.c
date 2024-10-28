//
// Created by why-iskra on 06.10.2024.
//

#include "domtree.h"
#include "morphine/utils/overflow.h"

#define GROW_FACTOR 8

#define elem(x)          ((struct elem) { .is_none = false, .value = (x) })
#define elem_none()      ((struct elem) { .is_none = true, .value = 0 })
#define elem_is_none(e)  ((e).is_none)
#define elem_value(U, e) ({ struct elem _e = (e); if (_e.is_none) mapi_error((U), "none element"); _e.value; })
#define elem_eq(a, b)    ({ struct elem _a = (a); struct elem _b = (b); (_a.is_none || _b.is_none) ? (_a.is_none == _b.is_none) : (_a.value == _b.value); })

struct elem {
    bool is_none;
    ml_size value;
};

struct bucket {
    size_t allocated;
    size_t size;
    ml_size *array;
};

struct domdata {
    ml_size count;
    ml_size *preorder;
    struct controlflow *domflow;
};

struct domtree {
    ml_size size;
    struct elem *idom;
    struct bucket *frontiers;
};

struct domtree *domtree_alloc(morphine_coroutine_t U) {
    struct domtree *domtree = mapi_allocator_uni(
        mapi_instance(U),
        NULL,
        sizeof(struct domtree)
    );

    (*domtree) = (struct domtree) {
        .size = 0,
        .idom = NULL,
        .frontiers = NULL
    };

    return domtree;
}

void domtree_free(morphine_instance_t I, struct domtree *domtree) {
    if (domtree != NULL) {
        if (domtree->frontiers != NULL) {
            for (ml_size i = 0; i < domtree->size; i++) {
                mapi_allocator_free(I, domtree->frontiers[i].array);
            }
        }

        mapi_allocator_free(I, domtree->idom);
        mapi_allocator_free(I, domtree->frontiers);
        mapi_allocator_free(I, domtree);
    }
}

static void domdata_free(morphine_instance_t I, void *data) {
    struct domdata *domdata = data;

    mapi_allocator_free(I, domdata->preorder);
    controlflow_free(I, domdata->domflow);
}

static ml_size *init_size(morphine_instance_t I, ml_size count) {
    ml_size *result = mapi_allocator_vec(I, NULL, count, sizeof(struct bucket));
    for (ml_size i = 0; i < count; i++) {
        result[i] = 0;
    }

    return result;
}

static struct elem *init_elem(morphine_instance_t I, ml_size count) {
    struct elem *result = mapi_allocator_vec(I, NULL, count, sizeof(struct elem));
    for (ml_size i = 0; i < count; i++) {
        result[i] = elem_none();
    }

    return result;
}

static struct bucket *init_bucket(morphine_instance_t I, ml_size count) {
    struct bucket *result = mapi_allocator_vec(I, NULL, count, sizeof(struct bucket));
    for (ml_size i = 0; i < count; i++) {
        result[i] = (struct bucket) {
            .allocated = 0,
            .size = 0,
            .array = NULL
        };
    }

    return result;
}

static struct domdata *push_data(morphine_coroutine_t U, ml_size count) {
    struct domdata *data = mapi_push_userdata_uni(U, sizeof(struct domdata));

    (*data) = (struct domdata) {
        .count = 0,
        .preorder = NULL,
        .domflow = NULL
    };

    mapi_userdata_set_free(U, domdata_free);

    morphine_instance_t I = mapi_instance(U);
    data->preorder = init_size(I, count);
    data->domflow = controlflow_alloc(U);
    data->count = count;

    return data;
}

static void bucket_add(morphine_coroutine_t U, struct bucket *bucket, ml_size value) {
    for (size_t i = 0; i < bucket->size; i++) {
        if (bucket->array[i] == value) {
            return;
        }
    }

    if (bucket->size == bucket->allocated) {
        overflow_add(bucket->allocated, GROW_FACTOR, SIZE_MAX) {
            mapi_error(U, "bucket overflow");
        }

        bucket->array = mapi_allocator_vec(
            mapi_instance(U),
            bucket->array,
            bucket->allocated + GROW_FACTOR,
            sizeof(ml_size)
        );

        bucket->allocated += GROW_FACTOR;
    }

    bucket->array[bucket->size] = value;
    bucket->size++;
}

static bool has_vertex(morphine_coroutine_t U, ml_size vertex) {
    ml_size size = mapi_vector_len(U);
    for (ml_size i = 0; i < size; i++) {
        mapi_vector_get(U, i);
        ml_size v = mapi_get_size(U, "vertex");
        mapi_pop(U, 1);

        if (v == vertex) {
            return true;
        }
    }

    return false;
}

static void build_front(
    morphine_coroutine_t U,
    struct domtree *domtree,
    struct domdata *data,
    struct controlflow *controlflow,
    ml_size start
) {
    controlflow_dfs(U, data->domflow, start);
    mapi_vector_remove(U, 0);
    mapi_pop(U, 1);

    ml_size size = mapi_vector_len(U);
    for (ml_size i = 0; i < size; i++) {
        mapi_vector_get(U, i);
        ml_size vertex = mapi_get_size(U, "vertex");
        mapi_pop(U, 1);

        controlflow_outgoing_edges(U, controlflow, vertex);

        ml_size outgoing_size = mapi_vector_len(U);
        for (ml_size j = 0; j < outgoing_size; j++) {
            mapi_vector_get(U, j);
            ml_size outgoing_vertex = mapi_get_size(U, "vertex");
            mapi_pop(U, 1);

            mapi_rotate(U, 2);
            if (!has_vertex(U, outgoing_vertex)) {
                bucket_add(U, domtree->frontiers + start, outgoing_vertex);
            }
            mapi_rotate(U, 2);
        }

        mapi_pop(U, 1);
    }

    {
        controlflow_outgoing_edges(U, controlflow, start);

        ml_size outgoing_size = mapi_vector_len(U);
        for (ml_size i = 0; i < outgoing_size; i++) {
            mapi_vector_get(U, i);
            ml_size outgoing_vertex = mapi_get_size(U, "vertex");
            mapi_pop(U, 1);

            mapi_rotate(U, 2);
            if (!has_vertex(U, outgoing_vertex)) {
                bucket_add(U, domtree->frontiers + start, outgoing_vertex);
            }
            mapi_rotate(U, 2);
        }

        mapi_pop(U, 1);
    }

    mapi_pop(U, 1);
}

static void build_frontiers(
    morphine_coroutine_t U,
    struct domtree *domtree,
    struct domdata *data,
    struct controlflow *controlflow
) {
    for (ml_size i = 0; i < domtree->size; i++) {
        struct elem idom = domtree->idom[i];
        if (elem_is_none(idom)) {
            continue;
        }

        controlflow_add_edge(U, data->domflow, elem_value(U, idom), i);
    }

    for (ml_size i = 0; i < domtree->size; i++) {
        build_front(U, domtree, data, controlflow, i);
    }
}

static struct elem intersect(
    morphine_coroutine_t U,
    struct domtree *domtree,
    struct domdata *data,
    ml_size a,
    struct elem b
) {
    struct elem t1 = elem(a);
    struct elem t2 = b;

    while (!elem_eq(t1, t2)) {
        if (data->preorder[elem_value(U, t1)] < data->preorder[elem_value(U, t2)]) {
            t2 = domtree->idom[elem_value(U, t2)];
        } else {
            t1 = domtree->idom[elem_value(U, t1)];
        }
    }

    return t1;
}

void domtree_build(
    morphine_coroutine_t U,
    struct blocks *blocks,
    struct controlflow *controlflow,
    struct domtree *domtree
) {
    struct domdata *data = push_data(U, blocks_size(blocks));

    {
        controlflow_dfs(U, controlflow, 0);
        ml_size size = mapi_vector_len(U);

        if (data->count != size) {
            mapi_error(U, "unused blocks");
        }

        for (ml_size i = 0; i < size; ++i) {
            mapi_vector_get(U, i);
            ml_size vertex = mapi_get_size(U, "vertex");
            mapi_pop(U, 1);

            if (vertex >= size) {
                mapi_error(U, "vertex out of bounce");
            }

            data->preorder[vertex] = i;
        }

        mapi_pop(U, 1);
    }

    {
        morphine_instance_t I = mapi_instance(U);
        domtree->idom = init_elem(I, data->count);
        domtree->frontiers = init_bucket(I, data->count);
        domtree->size = data->count;
    }

    if (data->count == 0) {
        mapi_error(U, "empty blocks");
    }

    ml_size firstelem = data->preorder[0];
    domtree->idom[data->preorder[0]] = elem(firstelem);

    bool changed;
    do {
        changed = false;

        for (ml_size i = 0; i < data->count; ++i) {
            ml_size node = data->preorder[i];

            if (node == firstelem) {
                continue;
            }

            struct elem oldidom = domtree->idom[node];
            struct elem newidom = elem_none();

            controlflow_incoming_edges(U, controlflow, node);
            ml_size size = mapi_vector_len(U);
            for (ml_size j = 0; j < size; j++) {
                mapi_vector_get(U, j);
                ml_size prenode = mapi_get_size(U, "vertex");
                mapi_pop(U, 1);

                if (elem_is_none(domtree->idom[prenode])) {
                    continue;
                }

                if (elem_is_none(newidom)) {
                    newidom = elem(prenode);
                } else {
                    newidom = intersect(U, domtree, data, prenode, newidom);
                }
            }
            mapi_pop(U, 1);

            if (elem_is_none(newidom)) {
                mapi_error(U, "none idom");
            }

            if (!elem_eq(newidom, oldidom)) {
                changed = true;
                domtree->idom[node] = newidom;
            }
        }
    } while (changed);

    build_frontiers(U, domtree, data, controlflow);

    mapi_pop(U, 1);
}

#include <stdio.h>

void domtree_dump(struct domtree *domtree) {
    printf("digraph D { ");
    for (ml_size i = 0; i < domtree->size; i++) {
        struct elem idom = domtree->idom[i];
        if (!elem_is_none(idom)) {
            printf("d%u -> d%u; ", idom.value, i);
        }
    }
    printf("}\n");

    printf("frontiers {\n");
    for (ml_size i = 0; i < domtree->size; i++) {
        struct bucket frontier = domtree->frontiers[i];
        printf("    %u = [", i);
        for (size_t j = 0; j < frontier.size; j++) {
            printf("%u", frontier.array[j]);
            if (j != frontier.size - 1) {
                printf(", ");
            }
        }
        printf("],\n");
    }
    printf("}\n");
}
