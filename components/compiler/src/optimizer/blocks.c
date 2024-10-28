//
// Created by why-iskra on 06.10.2024.
//

#include "blocks.h"

struct blocks {
    ml_size size;
    struct block *array;
};

struct blocks *blocks_alloc(morphine_coroutine_t U) {
    struct blocks *blocks = mapi_allocator_uni(
        mapi_instance(U),
        NULL,
        sizeof(struct blocks)
    );

    (*blocks) = (struct blocks) {
        .size = 0,
        .array = NULL
    };

    return blocks;
}

void blocks_free(morphine_instance_t I, struct blocks *blocks) {
    if (blocks != NULL) {
        mapi_allocator_free(I, blocks->array);
        mapi_allocator_free(I, blocks);
    }
}

static void insert_position(morphine_coroutine_t U, ml_size position, ml_size max) {
    if (position > max) {
        position = max;
    }

    ml_size size = mapi_vector_len(U);
    for (ml_size i = 0; i < size; i++) {
        mapi_vector_get(U, i);
        ml_size value = mapi_get_size(U, "position");
        mapi_pop(U, 1);

        if (value == position) {
            return;
        } else if (value > position) {
            mapi_push_size(U, position, "position");
            mapi_vector_add(U, i);
            return;
        }
    }

    mapi_push_size(U, position, "position");
    mapi_vector_add(U, size);
}

static void positions_vector(morphine_coroutine_t U, struct instructions *instructions) {
    ml_size size = instructions_size(instructions);

    mapi_push_vector(U, 0);
    mapi_vector_mode_fixed(U, false);

    insert_position(U, 0, size);
    insert_position(U, size, size);

    for (ml_size i = 0; i < size; i++) {
        morphine_instruction_t instruction = instructions_get(U, instructions, i).origin;
        switch (instruction.opcode) {
            case MORPHINE_OPCODE_JUMP:
                insert_position(U, i + 1, size);
                insert_position(U, instruction.argument1, size);
                break;
            case MORPHINE_OPCODE_JUMP_IF:
                insert_position(U, i + 1, size);
                insert_position(U, instruction.argument2, size);
                insert_position(U, instruction.argument3, size);
                break;
            case MORPHINE_OPCODE_LEAVE:
            case MORPHINE_OPCODE_RETURN:
                insert_position(U, i + 1, size);
                break;
            default:
                break;
        }
    }
}

void blocks_build(morphine_coroutine_t U, struct instructions *instructions, struct blocks *blocks) {
    positions_vector(U, instructions);

    ml_size size = mapi_vector_len(U);
    if (size < 2) {
        mapi_error(U, "blocks creation was failed");
    }

    blocks->array = mapi_allocator_vec(mapi_instance(U), NULL, size - 1, sizeof(struct block));
    blocks->size = size - 1;

    for (ml_size i = 0; i < (size - 1); i++) {
        mapi_vector_get(U, i);
        ml_size from = mapi_get_size(U, "position");
        mapi_pop(U, 1);

        mapi_vector_get(U, i + 1);
        ml_size to = mapi_get_size(U, "position");
        mapi_pop(U, 1);

        blocks->array[i] = (struct block) {
            .from = from,
            .to = to
        };
    }

    mapi_pop(U, 1);
}

void blocks_reformat(morphine_coroutine_t U, struct blocks *blocks) {
    mapi_vector_sort(U);
    ml_size size = mapi_vector_len(U);
    for (ml_size i = 0; i < size; i++) {
        mapi_vector_get(U, i);
        ml_size index = mapi_get_size(U, "block");
        mapi_pop(U, 1);

        blocks->array[i] = blocks->array[index];
    }
    mapi_pop(U, 1);

    blocks->array = mapi_allocator_vec(mapi_instance(U), blocks->array, size, sizeof(struct block));
    blocks->size = size;
}

ml_size blocks_size(struct blocks *blocks) {
    return blocks->size;
}

struct block blocks_get(morphine_coroutine_t U, struct blocks *blocks, ml_size index) {
    if (index >= blocks->size) {
        mapi_error(U, "block index out of bounce");
    }

    return blocks->array[index];
}

static ml_size get_block(struct blocks *blocks, ml_size index) {
    for (ml_size i = 0; i < blocks->size; i++) {
        struct block block = blocks->array[i];

        if (block.from >= index && index < block.to) {
            return i;
        }
    }

    return blocks->size;
}

struct blockedges blocks_edges(
    morphine_coroutine_t U,
    struct instructions *instructions,
    struct blocks *blocks,
    ml_size index
) {
    struct block block = blocks_get(U, blocks, index);
    if (block.to == block.from) {
        mapi_error(U, "empty block");
    }

    morphine_instruction_t instruction = instructions_get(U, instructions, block.to - 1).origin;
    switch (instruction.opcode) {
        case MORPHINE_OPCODE_JUMP:
            return (struct blockedges) {
                .count = 1,
                .edges[0] = get_block(blocks, instruction.argument1)
            };
        case MORPHINE_OPCODE_JUMP_IF:
            return (struct blockedges) {
                .count = 2,
                .edges[0] = get_block(blocks, instruction.argument2),
                .edges[1] = get_block(blocks, instruction.argument3)
            };
        case MORPHINE_OPCODE_LEAVE:
        case MORPHINE_OPCODE_RETURN:
            return (struct blockedges) {
                .count = 0,
            };
        default:
            return (struct blockedges) {
                .count = 1,
                .edges[0] = get_block(blocks, block.to)
            };
    }
}
