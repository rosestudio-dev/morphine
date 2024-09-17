//
// Created by why-iskra on 09.09.2024.
//

#include <string.h>
#include "controller.h"

struct collector {
    size_t data;
    size_t slots;
    size_t code;
    size_t anchors;
};

struct instruction_info {
    const char *name;
    morphine_opcode_t opcode;
    size_t args;
};

static struct instruction_info instructions[] = {
#define instruction(n, o, a)                    { #n, o, a },
#define mis_instruction_args0(n, s)             instruction(s, MORPHINE_OPCODE_##n, 0)
#define mis_instruction_args1(n, s, a1)         instruction(s, MORPHINE_OPCODE_##n, 1)
#define mis_instruction_args2(n, s, a1, a2)     instruction(s, MORPHINE_OPCODE_##n, 2)
#define mis_instruction_args3(n, s, a1, a2, a3) instruction(s, MORPHINE_OPCODE_##n, 3)

#include "morphine/instruction/specification.h"

#undef instruction
#undef mis_instruction_args0
#undef mis_instruction_args1
#undef mis_instruction_args2
#undef mis_instruction_args3
};

static struct instruction_info word2opcode(struct parse_controller *C, mc_strtable_index_t word) {
    struct mc_strtable_entry entry = parser_string(C, word);
    for (size_t i = 0; i < sizeof(instructions) / sizeof(struct instruction_info); i++) {
        if (entry.size != strlen(instructions[i].name)) {
            continue;
        }

        if (memcmp(entry.string, instructions[i].name, entry.size * sizeof(char)) == 0) {
            return instructions[i];
        }
    }

    parser_errorf(C, "undefined opcode");
}

static void build_data(
    struct parse_controller *C,
    struct mc_ast_expression_asm *result,
    struct collector *collector
) {
    struct mc_lex_token name_token = parser_consume(C, et_implicit_word());

    struct mc_asm_data data = {
        .name = name_token.word
    };

    if (parser_look(C, et_integer())) {
        struct mc_lex_token token = parser_consume(C, et_integer());
        data.type = MCADT_INTEGER;
        data.integer = token.integer;
    } else if (parser_look(C, et_decimal())) {
        struct mc_lex_token token = parser_consume(C, et_decimal());
        data.type = MCADT_DECIMAL;
        data.decimal = token.decimal;
    } else if (parser_look(C, et_string())) {
        struct mc_lex_token token = parser_consume(C, et_string());
        data.type = MCADT_STRING;
        data.string = token.string;
    } else if (parser_match(C, et_asm_predef_word(true))) {
        data.type = MCADT_BOOLEAN;
        data.boolean = true;
    } else if (parser_match(C, et_asm_predef_word(false))) {
        data.type = MCADT_BOOLEAN;
        data.boolean = false;
    } else if (parser_match(C, et_asm_predef_word(nil))) {
        data.type = MCADT_NIL;
    } else {
        parser_errorf(C, "expected integer, decimal, string, boolean or nil");
    }

    if (result != NULL) {
        result->data[collector->data] = data;
    }

    collector->data++;
}

static void build_slot(
    struct parse_controller *C,
    struct mc_ast_expression_asm *result,
    struct collector *collector
) {
    parser_consume(C, et_asm_predef_word(slot));
    struct mc_lex_token token = parser_consume(C, et_implicit_word());

    if (result != NULL) {
        result->slots[collector->slots] = token.word;
    }

    collector->slots++;
}

static void build_code(
    struct parse_controller *C,
    struct mc_ast_expression_asm *result,
    struct collector *collector
) {
    struct mc_lex_token opcode = parser_consume(C, et_implicit_word());
    if (parser_match(C, et_operator(COLON))) {
        struct mc_asm_anchor anchor = {
            .anchor = opcode.word,
            .instruction = collector->code
        };

        if (result != NULL) {
            result->anchors[collector->anchors] = anchor;
        }

        collector->anchors++;

        opcode = parser_consume(C, et_implicit_word());
    }

    struct instruction_info info = word2opcode(C, opcode.word);

    struct mc_asm_instruction instruction = {
        .line = opcode.line,
        .opcode = info.opcode
    };

    for (size_t i = 0; i < info.args; i++) {
        if (parser_look(C, et_integer())) {
            struct mc_lex_token token = parser_consume(C, et_integer());
            instruction.arguments[i] = (struct mc_asm_argument) {
                .type = MCAAT_NUMBER,
                .number = token.integer
            };
        } else if (parser_look(C, et_implicit_word())) {
            struct mc_lex_token token = parser_consume(C, et_implicit_word());
            instruction.arguments[i] = (struct mc_asm_argument) {
                .type = MCAAT_WORD,
                .word = token.word
            };
        } else {
            parser_errorf(C, "expected number or word");
        }

        if (i != info.args - 1) {
            parser_consume(C, et_operator(COMMA));
        }
    }

    size_t size = sizeof(instruction.arguments) / sizeof(instruction.arguments[0]);
    for (size_t i = info.args; i < size; i++) {
        instruction.arguments[i] = (struct mc_asm_argument) {
            .type = MCAAT_NUMBER,
            .number = 0
        };
    }

    if (result != NULL) {
        result->code[collector->code] = instruction;
    }

    collector->code++;
}

static void build_section(
    struct parse_controller *C,
    struct mc_ast_expression_asm *result,
    struct collector *collector
) {
    enum {
        DATA,
        SLOTS,
        CODE
    } type;

    if (parser_match(C, et_asm_predef_word(data))) {
        type = DATA;
    } else if (parser_match(C, et_asm_predef_word(slots))) {
        type = SLOTS;
    } else if (parser_match(C, et_asm_predef_word(code))) {
        type = CODE;
    } else {
        parser_errorf(C, "expected data, slots or code");
    }

    while (true) {
        if (parser_look(C, et_asm_predef_word(end))) {
            break;
        }

        if (parser_look(C, et_asm_predef_word(section))) {
            break;
        }

        switch (type) {
            case DATA:
                build_data(C, result, collector);
                break;
            case SLOTS:
                build_slot(C, result, collector);
                break;
            case CODE:
                build_code(C, result, collector);
                break;
        }
    }
}

static struct collector build(
    struct parse_controller *C,
    struct mc_ast_expression_asm *result
) {
    struct collector collector = {
        .data = 0,
        .slots = 0,
        .code = 0,
        .anchors = 0
    };

    parser_consume(C, et_predef_word(asm));

    parser_consume(C, et_operator(LPAREN));

    if (parser_look(C, et_word())) {
        struct mc_lex_token name = parser_consume(C, et_word());

        if (result != NULL) {
            result->has_emitter = true;
            result->emitter = name.word;
        }
    } else if (result != NULL) {
        result->has_emitter = false;
    }

    parser_consume(C, et_operator(RPAREN));

    parser_change_mode(C, PWM_ASM);

    while (parser_match(C, et_asm_predef_word(section))) {
        build_section(C, result, &collector);
    }

    parser_consume(C, et_asm_predef_word(end));

    return collector;
}

struct mc_ast_node *rule_asm(struct parse_controller *C) {
    struct collector collector = build(C, NULL);

    parser_reset(C);

    struct mc_ast_expression_asm *result = mcapi_ast_create_expression_asm(
        parser_U(C),
        parser_A(C),
        parser_get_line(C),
        collector.data,
        collector.slots,
        collector.code,
        collector.anchors
    );

    build(C, result);

    return mcapi_ast_expression_asm2node(result);
}
