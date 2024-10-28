//
// Created by why-iskra on 06.10.2024.
//

#pragma once

#include <morphine.h>
#include "controlflow.h"

struct domtree;

struct domtree *domtree_alloc(morphine_coroutine_t);
void domtree_free(morphine_instance_t, struct domtree *);
void domtree_build(morphine_coroutine_t, struct blocks *, struct controlflow *, struct domtree *);
void domtree_dump(struct domtree *);
