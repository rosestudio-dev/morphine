//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/proto.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/allocator.h"
#include <string.h>

bool protoI_uuid_equal(struct uuid a, struct uuid b) {
    return (a.most_significant_bits == b.most_significant_bits) &&
           (a.least_significant_bits == b.least_significant_bits);
}

struct proto *protoI_create(
    morphine_instance_t I,
    struct uuid uuid,
    size_t name_len,
    size_t constants_count,
    size_t instructions_count,
    size_t statics_count
) {
    size_t proto_size = sizeof(struct proto);
    size_t name_size = sizeof(char) * (name_len + 1);
    size_t constants_size = sizeof(struct value) * constants_count;
    size_t instructions_size = sizeof(instruction_t) * instructions_count;
    size_t statics_size = sizeof(struct value) * statics_count;

    size_t size = proto_size + name_size + constants_size + instructions_size + statics_size;

    struct proto *result = allocI_uni(I, NULL, size);

    void *ptr_name = ((void *) result) + proto_size;
    void *ptr_constants = ((void *) ptr_name) + name_size;
    void *ptr_instructions = ((void *) ptr_constants) + constants_size;
    void *ptr_statics = ((void *) ptr_instructions) + instructions_size;

    (*result) = (struct proto) {
        .uuid = uuid,
        .name = ptr_name,
        .name_len = name_len,
        .constants_count = constants_count,
        .instructions_count = instructions_count,
        .statics_count = statics_count,
        .arguments_count = 0,
        .slots_count = 0,
        .params_count = 0,
        .constants = ptr_constants,
        .instructions = ptr_instructions,
        .statics = ptr_statics,
        .registry_key = valueI_nil
    };

    for (size_t i = 0; i < constants_count; i++) {
        result->constants[i] = valueI_nil;
    }

    for (size_t i = 0; i < statics_count; i++) {
        result->statics[i] = valueI_nil;
    }

    result->name[name_len] = '\0';

    objectI_init(I, objectI_cast(result), OBJ_TYPE_PROTO);

    return result;
}

void protoI_free(morphine_instance_t I, struct proto *proto) {
    allocI_free(I, proto);
}

struct value protoI_static_get(morphine_instance_t I, struct proto *proto, size_t index) {
    if (proto == NULL) {
        throwI_error(I, "Proto is null");
    }

    if (index >= proto->statics_count) {
        throwI_error(I, "Static index was out of bounce");
    }

    return proto->statics[index];
}

void protoI_static_set(morphine_instance_t I, struct proto *proto, size_t index, struct value value) {
    if (proto == NULL) {
        throwI_error(I, "Proto is null");
    }

    if (index >= proto->statics_count) {
        throwI_error(I, "Static index was out of bounce");
    }

    gcI_barrier(proto, value);
    proto->statics[index] = value;
}


struct value protoI_constant_get(morphine_instance_t I, struct proto *proto, size_t index) {
    if (proto == NULL) {
        throwI_error(I, "Proto is null");
    }

    if (index >= proto->constants_count) {
        throwI_error(I, "Constant index was out of bounce");
    }

    return proto->constants[index];
}

void protoI_constant_set(morphine_instance_t I, struct proto *proto, size_t index, struct value value) {
    if (proto == NULL) {
        throwI_error(I, "Proto is null");
    }

    if (index >= proto->constants_count) {
        throwI_error(I, "Constant index was out of bounce");
    }

    gcI_barrier(proto, value);
    proto->constants[index] = value;
}
