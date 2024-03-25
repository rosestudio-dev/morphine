//
// Created by whyiskra on 16.12.23.
//

#include <setjmp.h>
#include <string.h>
#include "morphine/core/loader.h"
#include "morphine/core/crc32.h"
#include "morphine/object/proto.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/core/value.h"
#include "morphine/core/allocator.h"
#include "morphine/stack/access.h"

#define FORMAT_TAG      "morphine-bout"
#define FORMAT_TAG_SIZE (sizeof(FORMAT_TAG) - 1)

#define default_args struct loader_state *loader
#define default_params loader

struct loader_state {
    morphine_state_t S;
    struct crc32_buf crc;
    struct proto **created_protos;
    jmp_buf jump;
    void *data;
    morphine_loader_read_t read;
    const char *message;
};

static struct proto *loader_read(default_args);

struct proto *loaderI_load(
    morphine_state_t S,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args
) {
    struct loader_state loader = (struct loader_state) {
        .S = S,
        .read = read,
        .created_protos = NULL,
    };

    if (init == NULL) {
        loader.data = args;
    } else {
        loader.data = init(S, args);
    }

    size_t stack_size = stackI_space_size(S);

    struct proto *result;

    if (setjmp(loader.jump) != 0) {
        allocI_free(S->I, loader.created_protos);

        if (finish != NULL) {
            finish(S, loader.data);
        }

        throwI_message_error(S, loader.message);
    } else {
        result = loader_read(&loader);

        allocI_free(S->I, loader.created_protos);

        stackI_pop(S, stackI_space_size(S) - stack_size);

        if (finish != NULL) {
            finish(S, loader.data);
        }
    }

    return result;
}

morphine_noret static void loader_error(default_args, const char *message) {
    loader->message = message;
    longjmp(loader->jump, 1);
}

static uint8_t next_byte(default_args) {
    const char *error = NULL;
    uint8_t c = loader->read(loader->S, loader->data, &error);

    if (error != NULL) {
        loader_error(default_params, error);
    }

    crc32_char(&loader->crc, c);

    return c;
}

static uint8_t get_u8(default_args) {
    return next_byte(default_params);
}

static uint32_t get_u32(default_args) {
    char type = (char) get_u8(default_params);

    union {
        uint8_t raw[4];
        uint32_t result;
    } buffer;

    memset(buffer.raw, 0, 4);

    int init;
    switch (type) {
        case 'i':
            init = 0;
            break;
        case 'h':
            init = 1;
            break;
        case 's':
            init = 2;
            break;
        case 'b':
            init = 3;
            break;
        case 'z':
            init = 4;
            break;
        default:
            loader_error(default_params, "Unknown int tag");
    }

    for (int i = init; i < 4; i++) {
        buffer.raw[3 - i] = get_u8(default_params);
    }

    return buffer.result;
}

static uint64_t get_u64(default_args) {
    union {
        uint8_t raw[8];
        uint64_t result;
    } buffer;

    for (int i = 0; i < 8; i++) {
        buffer.raw[7 - i] = get_u8(default_params);
    }

    return buffer.result;
}

static double get_double(default_args) {
    union {
        uint64_t raw;
        double result;
    } buffer;

    buffer.raw = get_u64(default_params);

    return buffer.result;
}

static struct string *get_string(default_args) {
    size_t size = get_u32(default_params);
    char *buffer;
    struct string *string = stringI_createn(loader->S->I, size, &buffer);

    for (size_t i = 0; i < size; i++) {
        buffer[i] = (char) get_u8(default_params);
    }

    return string;
}

static struct uuid get_uuid(default_args) {
    uint64_t most_significant_bits = get_u64(default_params);
    uint64_t least_significant_bits = get_u64(default_params);

    return (struct uuid) {
        .most_significant_bits = most_significant_bits,
        .least_significant_bits = least_significant_bits
    };
}

static instruction_t loader_instruction(default_args) {
    instruction_t instruction;

    instruction.line = get_u32(default_params);
    instruction.opcode = get_u8(default_params);
    size_t args_count = get_u8(default_params);

    if (args_count > 3) {
        loader_error(default_params, "Instruction arguments corrupted");
    }

    argument_t *args = &instruction.argument1;

    for (size_t i = 0; i < args_count; i++) {
        char type = (char) get_u8(default_params);
        uint32_t value = get_u32(default_params);

        args[i].value = value;

        switch (type) {
            case 'c': {
                args[i].type = ARGUMENT_TYPE_CONSTANT;
                break;
            }
            case 's': {
                args[i].type = ARGUMENT_TYPE_SLOT;
                break;
            }
            case 'o': {
                args[i].type = ARGUMENT_TYPE_COUNT;
                break;
            }
            case 'p': {
                args[i].type = ARGUMENT_TYPE_PARAM;
                break;
            }
            case 'i': {
                args[i].type = ARGUMENT_TYPE_POSITION;
                break;
            }
            case 'l': {
                args[i].type = ARGUMENT_TYPE_CLOSURE;
                break;
            }
            case 't': {
                args[i].type = ARGUMENT_TYPE_STATIC;
                break;
            }
            case 'a': {
                args[i].type = ARGUMENT_TYPE_ARG;
                break;
            }

            default:
                loader_error(default_params, "Unsupported argument tag");
        }
    }

    for (size_t i = args_count; i < 3; i++) {
        args[i] = (argument_t) {
            .value = 0,
            .type = ARGUMENT_TYPE_UNDEFINED
        };
    }

    return instruction;
}

static void validate_instruction(
    default_args,
    instruction_t instruction,
    size_t arguments_count,
    size_t slots_count,
    size_t params_count,
    size_t closures_count,
    size_t statics_count,
    size_t constants_count
) {
    bool is_valid = instructionI_validate(
        instruction,
        arguments_count,
        slots_count,
        params_count,
        closures_count,
        statics_count,
        constants_count
    );

    if (!is_valid) {
        loader_error(default_params, "Instruction structure corrupted");
    }
}

static struct proto *loader_function(default_args) {
    struct uuid uuid = get_uuid(default_params);
    size_t name_chars_count = get_u32(default_params);
    size_t arguments_count = get_u32(default_params);
    size_t slots_count = get_u32(default_params);
    size_t params_count = get_u32(default_params);
    size_t closures_count = get_u32(default_params);
    size_t statics_count = get_u32(default_params);
    size_t constants_count = get_u32(default_params);
    size_t instructions_count = get_u32(default_params);

    struct proto *proto = protoI_create(
        loader->S->I,
        uuid,
        name_chars_count,
        constants_count,
        instructions_count,
        statics_count
    );
    stackI_push(loader->S, valueI_object(proto));

    for (size_t i = 0; i < instructions_count; i++) {
        proto->instructions[i] = loader_instruction(default_params);
    }

    for (size_t i = 0; i < instructions_count; i++) {
        validate_instruction(
            default_params,
            proto->instructions[i],
            arguments_count,
            slots_count,
            params_count,
            closures_count,
            statics_count,
            constants_count
        );
    }

    proto->arguments_count = arguments_count;
    proto->slots_count = slots_count;
    proto->closures_count = closures_count;
    proto->params_count = params_count;

    return proto;
}

static struct value loader_constant(default_args, struct proto **protos, size_t protos_count) {
    char type = (char) get_u8(default_params);

    switch (type) {
        case 'b': {
            return valueI_boolean(get_u8(default_params));
        }
        case 'i': {
            return valueI_integer((morphine_integer_t) get_u32(default_params));
        }
        case 'f': {
            struct uuid uuid = get_uuid(default_params);

            struct proto *found = NULL;
            for (size_t i = 0; i < protos_count; i++) {
                if (protoI_uuid_equal(protos[i]->uuid, uuid)) {
                    found = protos[i];
                    break;
                }
            }

            if (found == NULL) {
                loader_error(default_params, "Function constant corrupted");
            }

            return valueI_object(found);
        }
        case 'd': {
            return valueI_decimal((morphine_decimal_t) get_double(default_params));
        }
        case 's': {
            struct value string = valueI_object(get_string(default_params));
            stackI_push(loader->S, string);
            return string;
        }
        case 'n': {
            return valueI_nil;
        }

        default: {
            loader_error(default_params, "Unsupported constant tag");
        }
    }
}

static void loader_constants(default_args, struct proto *proto, size_t protos_count) {
    for (size_t i = 0; i < proto->constants_count; i++) {
        protoI_constant_set(
            loader->S, proto, i,
            loader_constant(default_params, loader->created_protos, protos_count)
        );
    }
}

static void loader_name(default_args, struct proto *proto) {
    for (size_t i = 0; i < proto->name_chars_count; i++) {
        proto->name[i] = (char) get_u8(default_params);
    }
}

static void check_csum(default_args) {
    uint32_t expected = crc32_result(&loader->crc);
    uint32_t hash = get_u32(default_params);
    if (expected != hash) {
        loader_error(default_params, "Binary corrupted");
    }
}

static void check_tag(default_args) {
    char buffer[FORMAT_TAG_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));

    for (size_t i = 0; i < FORMAT_TAG_SIZE; i++) {
        buffer[i] = (char) get_u8(default_params);
    }

    if (strcmp(buffer, FORMAT_TAG) != 0) {
        loader_error(default_params, "Wrong format tag");
    }
}

static struct proto *loader_read(default_args) {
    // read

    loader->crc = crc32_init();

    check_tag(default_params);

    struct uuid main_function_id = get_uuid(default_params);
    size_t functions_count = get_u32(default_params);

    loader->created_protos = allocI_uni(loader->S->I, NULL, functions_count * sizeof(struct proto *));

    for (size_t i = 0; i < functions_count; i++) {
        loader->created_protos[i] = loader_function(default_params);
    }

    for (size_t i = 0; i < functions_count; i++) {
        loader_constants(default_params, loader->created_protos[i], functions_count);
    }

    for (size_t i = 0; i < functions_count; i++) {
        loader_name(default_params, loader->created_protos[i]);
    }

    check_csum(default_params);

    // resolve

    struct proto *main = NULL;

    for (size_t i = 0; i < functions_count; i++) {
        if (protoI_uuid_equal(loader->created_protos[i]->uuid, main_function_id)) {
            main = loader->created_protos[i];
            break;
        }
    }

    if (main == NULL) {
        loader_error(default_params, "Main callable wasn't found");
    }

    return main;
}
