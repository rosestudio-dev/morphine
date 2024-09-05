//
// Created by why-iskra on 15.08.2024.
//

#pragma once

#include "morphinec/serializer.h"

struct serializer_controller;

morphine_noret void serializer_enter_expression(
    struct serializer_controller *,
    struct mc_ast_expression *,
    size_t next_state
);

morphine_noret void serializer_enter_statement(
    struct serializer_controller *,
    struct mc_ast_statement *,
    size_t next_state
);

morphine_noret void serializer_enter_function(
    struct serializer_controller *,
    struct mc_ast_function *,
    size_t next_state
);

morphine_noret void serializer_jump(
    struct serializer_controller *,
    size_t next_state
);

morphine_noret void serializer_complete(struct serializer_controller *);

void *serializer_saved(struct serializer_controller *);
void *serializer_alloc_saved_uni(struct serializer_controller *, size_t);

void serializer_push_string(struct serializer_controller *, mc_strtable_index_t);
void serializer_push_cstr(struct serializer_controller *, const char *);
void serializer_push_integer(struct serializer_controller *, ml_integer);
void serializer_push_decimal(struct serializer_controller *, ml_decimal);
void serializer_push_boolean(struct serializer_controller *, bool);
void serializer_push_size(struct serializer_controller *, size_t, const char *);
void serializer_push_vector(struct serializer_controller *, size_t);
void serializer_vector_set(struct serializer_controller *, size_t);
void serializer_push_table(struct serializer_controller *);
void serializer_set(struct serializer_controller *, const char *);
