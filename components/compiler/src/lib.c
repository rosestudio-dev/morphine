//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/lib.h"
#include "morphinec/compiler.h"
#include "morphinec/disassembler.h"
#include "morphinec/binary.h"
#include "morphinec/rollout.h"
#include "morphinec/strtable.h"
#include "morphinec/ast.h"
#include "morphinec/lex.h"
#include "morphinec/parser.h"
#include "morphinec/visitor.h"
#include "morphinec/codegen.h"

#define DEFAULT_MAIN_NAME     "compiled"
#define DEFAULT_MAIN_NAME_LEN ((sizeof(DEFAULT_MAIN_NAME) / sizeof(char)) - 1)

#define STR_KEY "str"
#define AST_KEY "ast"
#define LEX_KEY "mcapi_push_lex"
#define PRS_KEY "prs"
#define VST_KEY "vst"
#define CDG_KEY "cdg"

static void compile(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            const char *main;
            size_t main_len;
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 0);
                main = mapi_get_string(U);
                main_len = mapi_string_len(U);
                mapi_push_arg(U, 1);
            } else {
                maux_expect_args(U, 1);
                main = DEFAULT_MAIN_NAME;
                main_len = DEFAULT_MAIN_NAME_LEN;
                mapi_push_arg(U, 0);
            }

            const char *text = mapi_get_string(U);
            size_t text_len = mapi_string_len(U);

            mcapi_compile(U, main, text, main_len, text_len);
            maux_nb_return();
    maux_nb_end
}

static void distributed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            const char *main;
            size_t main_len;
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 0);
                main = mapi_get_string(U);
                main_len = mapi_string_len(U);
                mapi_push_arg(U, 1);
            } else {
                maux_expect_args(U, 1);
                main = DEFAULT_MAIN_NAME;
                main_len = DEFAULT_MAIN_NAME_LEN;
                mapi_push_arg(U, 0);
            }

            const char *text = mapi_get_string(U);
            size_t text_len = mapi_string_len(U);

            mapi_bind_registry(U);

            mapi_push_string(U, STR_KEY);
            struct mc_strtable *T = mcapi_push_strtable(U);
            mapi_registry_set(U);

            mapi_push_string(U, AST_KEY);
            mc_strtable_index_t name = mcapi_strtable_record(U, T, main, main_len);
            struct ast *A = ast(U, name);
            mapi_registry_set(U);

            mapi_push_string(U, LEX_KEY);
            struct mc_lex *L = mcapi_push_lex(U, text, text_len);
            mapi_registry_set(U);

            mapi_push_string(U, PRS_KEY);
            parser(U, T, L, A);
            mapi_registry_set(U);

            mapi_stack_reset(U);
            maux_nb_continue(1);

        maux_nb_state(1)
            mapi_push_string(U, PRS_KEY);
            mapi_registry_get(U);

            struct parser *P = get_parser(U);
            bool cont = parser_step(U, P);
            mapi_stack_reset(U);

            if (cont) {
                maux_nb_continue(1);
            } else {
                maux_nb_continue(2);
            }

        maux_nb_state(2)
            mapi_push_string(U, LEX_KEY);
            mapi_registry_remove(U);
            mapi_push_string(U, PRS_KEY);
            mapi_registry_remove(U);
            mapi_pop(U, 2);

            mapi_push_string(U, STR_KEY);
            mapi_registry_get(U);
            struct mc_strtable *T = mcapi_get_strtable(U);

            mapi_push_string(U, AST_KEY);
            mapi_registry_get(U);
            struct ast *A = get_ast(U);

            mapi_push_string(U, VST_KEY);
            struct visitor *V = visitor(U, A);
            mapi_registry_set(U);

            mapi_push_string(U, CDG_KEY);
            codegen(U, T, A, V);
            mapi_registry_set(U);

            mapi_stack_reset(U);
            maux_nb_continue(3);

        maux_nb_state(3)
            mapi_push_string(U, CDG_KEY);
            mapi_registry_get(U);

            struct codegen *C = get_codegen(U);
            bool cont = codegen_step(U, C);
            mapi_stack_reset(U);

            if (cont) {
                maux_nb_continue(3);
            } else {
                maux_nb_continue(4);
            }

        maux_nb_state(4)
            mapi_push_string(U, CDG_KEY);
            mapi_registry_get(U);

            struct codegen *C = get_codegen(U);
            codegen_construct(U, C);
            maux_nb_return();
    maux_nb_end
}

static void disassembly(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);

            ml_size count;
            if (mapi_is_type(U, "vector")) {
                count = mapi_vector_len(U);
            } else {
                mapi_push_vector(U, 1);
                mapi_rotate(U, 2);
                mapi_vector_set(U, 0);
                count = 1;
            }

            for (ml_size i = 0; i < count; i++) {
                mapi_vector_get(U, i);
                mapi_push_arg(U, 0);
                mapi_rotate(U, 2);

                mapi_extract_callable(U);
                mapi_rotate(U, 2);
                mapi_pop(U, 1);

                mcapi_disassembly(U);

                mapi_pop(U, 1);
                mapi_sio_print(U, "\n");
                mapi_pop(U, 1);
            }
            maux_nb_leave();
    maux_nb_end
}

static void tobinary(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);

            mapi_push_arg(U, 1);
            mapi_extract_callable(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            mcapi_to_binary(U);
            maux_nb_leave();
    maux_nb_end
}

static void frombinary(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mcapi_from_binary(U);
            maux_nb_return();
    maux_nb_end
}

static void rollout(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mcapi_rollout_as_vector(U);
            maux_nb_return();
    maux_nb_end
}

static void strtable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mcapi_push_strtable(U);
            maux_nb_return();
    maux_nb_end
}

static void strtableaccess(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, MC_STRTABLE_USERDATA_TYPE);

            struct mc_strtable *T = mcapi_get_strtable(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "index");
            ml_size index = mapi_get_index(U);

            if (mcapi_strtable_has(T, index)) {
                struct mc_strtable_entry entry = mcapi_strtable_access(U, T, index);
                mapi_push_stringn(U, entry.string, entry.size);
            } else {
                mapi_push_nil(U);
            }

            maux_nb_return();
    maux_nb_end
}

static void lex(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "string");

            const char *text = mapi_get_string(U);
            ml_size size = mapi_string_len(U);
            mcapi_push_lex(U, text, size);
            maux_nb_return();
    maux_nb_end
}

static void lexstep(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, MC_LEX_USERDATA_TYPE);

            struct mc_lex *L = mcapi_get_lex(U);

            mapi_push_arg(U, 1);
            maux_expect(U, MC_STRTABLE_USERDATA_TYPE);

            struct mc_strtable *T = mcapi_get_strtable(U);

            struct mc_lex_token token = mcapi_lex_step(U, L, T);

            mapi_push_table(U);

            mapi_push_string(U, "type");
            mapi_push_string(U, mcapi_lex_type2str(U, token.type));
            mapi_table_set(U);

            mapi_push_string(U, "data");
            switch (token.type) {
                case MCLTT_EOS:
                    mapi_push_nil(U);
                    break;
                case MCLTT_INTEGER:
                    mapi_push_integer(U, token.integer);
                    break;
                case MCLTT_DECIMAL:
                    mapi_push_decimal(U, token.decimal);
                    break;
                case MCLTT_STRING:
                    mapi_push_size(U, token.string);
                    break;
                case MCLTT_WORD:
                    mapi_push_size(U, token.word);
                    break;
                case MCLTT_PREDEFINED_WORD:
                    mapi_push_string(U, mcapi_lex_predefined2str(U, token.predefined_word));
                    break;
                case MCLTT_OPERATOR:
                    mapi_push_string(U, mcapi_lex_operator2name(U, token.operator));
                    break;
                case MCLTT_COMMENT:
                    mapi_push_size(U, token.comment);
                    break;
                default:
                    mapi_push_nil(U);
                    break;
            }
            mapi_table_set(U);

            mapi_push_string(U, "line");
            mapi_push_size(U, token.line);
            mapi_table_set(U);

            mapi_push_string(U, "range");
            mapi_push_table(U);

            mapi_push_string(U, "from");
            mapi_push_size(U, token.range.from);
            mapi_table_set(U);

            mapi_push_string(U, "to");
            mapi_push_size(U, token.range.to);
            mapi_table_set(U);

            mapi_table_set(U);
            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "compile",        compile },
    { "distributed",    distributed },
    { "disassembly",    disassembly },
    { "tobinary",       tobinary },
    { "frombinary",     frombinary },
    { "rollout",        rollout },

    { "strtable",       strtable },
    { "strtableaccess", strtableaccess },
    { "lex",            lex },
    { "lexstep",        lexstep },

    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "compiler",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mclib_compiler(void) {
    return &library;
}
