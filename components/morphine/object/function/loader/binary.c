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

static uint16_t get_u16(struct data *data) {
    union {
        uint8_t raw[2];
        uint16_t result;
    } buffer;

    buffer.raw[1] = get_u8(data);
    buffer.raw[0] = get_u8(data);

    return buffer.result;
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

    buffer.raw[7] = get_u8(data);
    buffer.raw[6] = get_u8(data);
    buffer.raw[5] = get_u8(data);
    buffer.raw[4] = get_u8(data);

    buffer.raw[3] = get_u8(data);
    buffer.raw[2] = get_u8(data);
    buffer.raw[1] = get_u8(data);
    buffer.raw[0] = get_u8(data);

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

static struct function *get_function(struct data *data) {
    struct uuid uuid = get_uuid(data);
    size_t name_len = get_u16(data);
    size_t arguments_count = get_u16(data);
    size_t slots_count = get_u16(data);
    size_t params_count = get_u16(data);
    size_t closures_count = get_u16(data);
    size_t statics_count = get_u16(data);
    size_t constants_count = get_u16(data);
    size_t instructions_count = get_u16(data);

    struct function *function = functionI_create(
        data->U->I,
        uuid,
        name_len,
        constants_count,
        instructions_count,
        statics_count,
        arguments_count,
        slots_count,
        closures_count,
        params_count
    );

    stackI_push(data->U, valueI_object(function));

    return function;
}

static void load_instructions(struct data *data, struct function *function) {
    for (size_t i = 0; i < function->instructions_count; i++) {
        instruction_t instruction = {
            .line = 0,
            .opcode = get_u8(data),
            .argument1.value = 0,
            .argument2.value = 0,
            .argument3.value = 0,
        };

        if (!instructionI_is_valid_opcode(instruction.opcode)) {
            process_error(data->state, "Unsupported opcode");
        }

        argument_t *args = &instruction.argument1;
        size_t count = instructionI_opcode_args[instruction.opcode];
        for (size_t c = 0; c < count; c++) {
            args[c].value = get_u16(data);
        }

        function->instructions[i] = instruction;
    }

    functionI_validate(data->U->I, function);
}

static void load_lines(struct data *data, struct function *function) {
    for (size_t i = 0; i < function->instructions_count; i++) {
        function->instructions[i].line = get_u32(data);
    }
}

static void load_constants(struct data *data, struct function *function) {
    for (size_t i = 0; i < function->constants_count; i++) {
        char type = (char) get_u8(data);

        struct value constant = valueI_nil;
        switch (type) {
            case 'b': {
                constant = valueI_boolean(get_u8(data));
                break;
            }
            case 'i': {
                constant = valueI_integer((ml_integer) get_u32(data));
                break;
            }
            case 'f': {
                struct uuid uuid = get_uuid(data);

                struct function *found = NULL;
                for (size_t c = 0; c < data->functions.count; c++) {
                    if (uuidI_equal(data->functions.vector[c]->uuid, uuid)) {
                        found = data->functions.vector[c];
                        break;
                    }
                }

                if (found == NULL) {
                    process_error(data->state, "Function constant corrupted");
                }

                constant = valueI_object(found);
                break;
            }
            case 'd': {
                constant = valueI_decimal((ml_decimal) get_double(data));
                break;
            }
            case 's': {
                constant = get_string(data);
                break;
            }
            case 'n': {
                constant = valueI_nil;
                break;
            }

            default: {
                process_error(data->state, "Unsupported constant tag");
            }
        }

        functionI_constant_set(
            data->U->I,
            function,
            i,
            constant
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

    struct userdata *userdata = userdataI_create_vec(
        data->U->I, "functions", functions_count, sizeof(struct function *), NULL
    );

    stackI_push(data->U, valueI_object(userdata));

    struct function **functions = userdata->data;
    data->functions.count = functions_count;
    data->functions.vector = functions;

    for (size_t i = 0; i < functions_count; i++) {
        functions[i] = get_function(data);
    }

    for (size_t i = 0; i < functions_count; i++) {
        load_instructions(data, functions[i]);
    }

    for (size_t i = 0; i < functions_count; i++) {
        load_lines(data, functions[i]);
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

static struct function *get_main(struct data *data, struct uuid main_uuid) {
    struct function *main = NULL;

    for (size_t i = 0; i < data->functions.count; i++) {
        if (uuidI_equal(data->functions.vector[i]->uuid, main_uuid)) {
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
    return get_main(&data, main_uuid);
}
