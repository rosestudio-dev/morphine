//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "value.h"

struct hashmap;

struct pair {
    struct value key;
    struct value value;
};

struct hashmap *hashmap_new(morphine_instance_t, size_t cap);

void hashmap_free(morphine_instance_t, struct hashmap *map);
void hashmap_clear(morphine_instance_t, struct hashmap *map, bool update_cap);
size_t hashmap_count(struct hashmap *map);
size_t hashmap_allocated_size(struct hashmap *map);
bool hashmap_iter(struct hashmap *map, size_t *i, struct pair *item);

bool hashmap_set(morphine_instance_t, struct hashmap *map, struct pair item);
struct value hashmap_get(morphine_instance_t, struct hashmap *map, struct value key, bool *has);
bool hashmap_delete(morphine_instance_t, struct hashmap *map, struct value key);
