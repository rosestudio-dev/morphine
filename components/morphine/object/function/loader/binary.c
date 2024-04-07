//
// Created by why-iskra on 31.03.2024.
//

#include "binary.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/function.h"
#include "morphine/object/string.h"
#include "morphine/object/userdata.h"
#include "morphine/gc/allocator.h"
#include <string.h>

struct data {
    morphine_coroutine_t U;
    struct process_state *state;
    struct crc32_buf crc;

    struct {
        struct function **vector;
        size_t count;
    } functions;
};

static uint8_t get_u8(struct data *data) {
    uint8_t byte = process_byte(data->state);
    crc32_char(&data->crc, byte);
    return byte;
}

static uint32_t get_u32(struct data *data) {
    char type = (char) get_u8(data);

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
            process_error(data->state, "Unknown int tag");
    }

    for (int i = init; i < 4; i++) {
        buffer.raw[3 - i] = get_u8(data);
    }

    return buffer.result;
}

static uint64_t get_u64(struct data *data) {
    union {
        uint8_t raw[8];
        uint64_t result;
    } buffer;

    for (int i = 0; i < 8; i++) {
        buffer.raw[7 - i] = get_u8(data);
    }

    return buffer.result;
}

static double get_double(struct data *data) {
    union {
        uint64_t raw;
        double result;
    } buffer;

    buffer.raw = get_u64(data);

    return buffer.result;
}

static char get_char(struct data *data) {
    union {
        uint8_t raw[sizeof(char)];
        char result;
    } buffer;

    for (size_t i = 0; i < sizeof(char); i++) {
        buffer.raw[i] = get_u8(data);
    }

    return buffer.result;
}

static struct value get_string(struct data *data) {
    size_t size = get_u32(data);
    char *buffer;
    struct string *string = stringI_createn(data->U->I, size, &buffer);

    for (size_t i = 0; i < size; i++) {
        buffer[i] = get_char(data);
    }

    struct value value = valueI_object(string);
    stackI_push(data->U, value);

    return value;
}

static struct uuid get_uuid(struct data *data) {
    uint64_t most_significant_bits = get_u64(data);
    uint64_t least_significant_bits = get_u64(data);

    return (struct uuid) {
        .most_significant_bits = most_significant_bits,
        .least_significant_bits = least_significant_bits
    };
}

static instruction_t get_instruction(struct data *data) {
    instruction_t instruction;

    instruction.line = get_u32(data);
    instruction.opcode = get_u8(data);
    size_t args_count = get_u8(data);

    if (args_count > 3) {
        process_error(data->state, "Instruction arguments corrupted");
    }

    argument_t *args = &instruction.argument1;

    for (size_t i = 0; i < args_count; i++) {
        char type = (char) get_u8(data);
        uint32_t value = get_u32(data);

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
                process_error(data->state, "Unsupported argument tag");
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

static struct function *get_function(struct data *data) {
    struct uuid uuid = get_uuid(data);
    size_t name_chars_count = get_u32(data);
    size_t arguments_count = get_u32(data);
    size_t slots_count = get_u32(data);
    size_t params_count = get_u32(data);
    size_t closures_count = get_u32(data);
    size_t statics_count = get_u32(data);
    size_t constants_count = get_u32(data);
    size_t instructions_count = get_u32(data);

    struct function *function = functionI_create(
        data->U->I,
        uuid,
        name_chars_count,
        constants_count,
        instructions_count,
        statics_count,
        arguments_count,
        slots_count,
        closures_count,
        params_count
    );

    stackI_push(data->U, valueI_object(function));

    for (size_t i = 0; i < instructions_count; i++) {
        function->instructions[i] = get_instruction(data);
    }

    functionI_validate(data->U->I, function);

    return function;
}

static struct value get_constant(struct data *data) {
    char type = (char) get_u8(data);

    switch (type) {
        case 'b': {
            return valueI_boolean(get_u8(data));
        }
        case 'i': {
            return valueI_integer((ml_integer) get_u32(data));
        }
        case 'f': {
            struct uuid uuid = get_uuid(data);

            struct function *found = NULL;
            for (size_t i = 0; i < data->functions.count; i++) {
                if (functionI_uuid_equal(data->functions.vector[i]->uuid, uuid)) {
                    found = data->functions.vector[i];
                    break;
                }
            }

            if (found == NULL) {
                process_error(data->state, "Function constant corrupted");
            }

            return valueI_object(found);
        }
        case 'd': {
            return valueI_decimal((ml_decimal) get_double(data));
        }
        case 's': {
            return get_string(data);
        }
        case 'n': {
            return valueI_nil;
        }

        default: {
            process_error(data->state, "Unsupported constant tag");
        }
    }
}

static void load_constants(struct data *data, struct function *function) {
    for (size_t i = 0; i < function->constants_count; i++) {
        functionI_constant_set(
            data->U->I,
            function,
            i,
            get_constant(data)
        );
    }
}

static void load_name(struct data *data, struct function *function) {
    for (size_t i = 0; i < function->name_len; i++) {
        function->name[i] = get_char(data);
    }
}

static void check_csum(struct data *data) {
    uint32_t expected = crc32_result(&data->crc);
    uint32_t hash = get_u32(data);
    if (expected != hash) {
        process_error(data->state, "Binary corrupted");
    }
}

static void check_tag(struct data *data) {
    char buffer[sizeof(FORMAT_TAG)];
    memset(buffer, 0, sizeof(buffer));

    for (size_t i = 0; i < (sizeof(FORMAT_TAG) - 1); i++) {
        buffer[i] = get_char(data);
    }

    if (strcmp(buffer, FORMAT_TAG) != 0) {
        process_error(data->state, "Wrong format tag");
    }
}

static struct uuid load(struct data *data) {
    check_tag(data);

    struct uuid main_uuid = get_uuid(data);
    size_t functions_count = get_u32(data);

    struct function **functions = allocI_uni(data->U->I, NULL, functions_count * sizeof(struct function *));
    data->functions.count = functions_count;
    data->functions.vector = functions;

    struct userdata *userdata = userdataI_create(data->U->I, "functions", functions, NULL, allocI_free);
    stackI_push(data->U, valueI_object(userdata));

    for (size_t i = 0; i < functions_count; i++) {
        functions[i] = get_function(data);
    }

    for (size_t i = 0; i < functions_count; i++) {
        load_constants(data, functions[i]);
    }

    for (size_t i = 0; i < functions_count; i++) {
        load_name(data, functions[i]);
    }

    check_csum(data);

    return main_uuid;
}

static struct function *resolve(struct data *data, struct uuid main_uuid) {
    struct function *main = NULL;

    for (size_t i = 0; i < data->functions.count; i++) {
        if (functionI_uuid_equal(data->functions.vector[i]->uuid, main_uuid)) {
            main = data->functions.vector[i];
            break;
        }
    }

    if (main == NULL) {
        process_error(data->state, "Main function wasn't found");
    }

    return main;
}

struct function *binary(morphine_coroutine_t U, struct process_state *state) {
    struct data data = {
        .U = U,
        .state = state,
        .crc = crc32_init(),
        .functions.vector = NULL,
        .functions.count = 0
    };

    struct uuid main_uuid = load(&data);
    return resolve(&data, main_uuid);
}
