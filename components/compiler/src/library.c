//
// Created by why-iskra on 08.06.2024.
//

#include "morphinec/library.h"
#include "morphinec/compiler.h"
#include "morphinec/disassembler.h"
#include "morphinec/lex.h"
#include "morphinec/ast.h"
#include "morphinec/parser.h"
#include "morphinec/visitor.h"
#include "morphinec/codegen.h"
#include "morphinec/serializer.h"

#define DEFAULT_MAIN_NAME "compiled"

static void compile(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            bool vector = false;
            bool yieldable = false;
            const char *name = DEFAULT_MAIN_NAME;
            if (mapi_is_type(U, "table")) {
                mapi_push_string(U, "name");
                if (mapi_table_get(U)) {
                    name = mapi_get_cstr(U);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "vector");
                if (mapi_table_get(U)) {
                    vector = mapi_get_boolean(U);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "yieldable");
                if (mapi_table_get(U)) {
                    yieldable = mapi_get_boolean(U);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "text");
                mapi_table_get(U);
            }

            if (!yieldable) {
                mcapi_compile(U, name, vector);
                maux_nb_return();
            } else {
                mapi_bind_registry(U);

                const char *text = mapi_get_string(U);
                ml_size text_len = mapi_string_len(U);

                mapi_push_boolean(U, vector);
                maux_registry_set(U, "vector");

                mapi_push_string(U, name);
                maux_registry_set(U, "name");

                mcapi_push_strtable(U);
                maux_registry_set(U, "T");

                mcapi_push_ast(U);
                maux_registry_set(U, "A");

                mcapi_push_lex(U, text, text_len);
                maux_registry_set(U, "LV");

                mcapi_push_parser(U);
                maux_registry_set(U, "PG");

                maux_nb_continue(1);
            }
        maux_nb_state(1)
            maux_registry_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_registry_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_registry_get(U, "LV");
            struct mc_lex *L = mcapi_get_lex(U);

            maux_registry_get(U, "PG");
            struct mc_parser *P = mcapi_get_parser(U);

            bool next = mcapi_parser_step(U, P, A, L, T);
            mapi_pop(U, 4);

            if (next) {
                maux_nb_continue(1);
            } else {
                mcapi_push_visitor(U);
                maux_registry_set(U, "LV");

                mcapi_push_codegen(U);
                maux_registry_set(U, "PG");

                maux_nb_continue(2);
            }
        maux_nb_state(2)
            maux_registry_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_registry_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_registry_get(U, "LV");
            struct mc_visitor *V = mcapi_get_visitor(U);

            maux_registry_get(U, "PG");
            struct mc_codegen *G = mcapi_get_codegen(U);

            if (mcapi_codegen_step(U, G, V, T, A)) {
                mapi_pop(U, 4);
                maux_nb_continue(2);
            }

            maux_registry_get(U, "vector");
            bool vector = mapi_get_boolean(U);

            maux_registry_get(U, "name");
            const char *name = mapi_get_cstr(U);

            mcapi_codegen_build(U, G, T, A, name, vector);
            maux_nb_return();
    maux_nb_end
}

static void disassembly(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_table(U);

            mapi_push_arg(U, 1);
            mapi_extract_callable(U);
            mapi_rotate(U, 2);
            mapi_pop(U, 1);

            mapi_push_nil(U);
            mapi_table_set(U);

            for (ml_size index = 0; index < mapi_table_len(U); index++) {
                mapi_table_idx_keyoe(U, index);
                for (ml_size i = 0; i < mapi_constant_size(U); i++) {
                    mapi_constant_get(U, i);

                    if (mapi_is_type(U, "function")) {
                        mapi_peek(U, 2);
                        mapi_rotate(U, 2);
                        mapi_push_nil(U);
                        mapi_table_set(U);
                    }

                    mapi_pop(U, 1);
                }

                mapi_push_arg(U, 0);
                mapi_rotate(U, 2);

                mcapi_disassembly(U);

                if (index != mapi_table_len(U) - 1) {
                    mapi_push_arg(U, 0);
                    mapi_sio_print(U, "\n");
                    mapi_pop(U, 1);
                }
            }
            maux_nb_leave();
    maux_nb_end
}

static void strtable_create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mcapi_push_strtable(U);
            maux_nb_return();
    maux_nb_end
}

static void strtable_access(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            struct mc_strtable *strtable = mcapi_get_strtable(U);
            mapi_push_arg(U, 1);
            ml_size index = mapi_get_size(U, "index");

            if (mcapi_strtable_has(strtable, index)) {
                struct mc_strtable_entry entry = mcapi_strtable_access(U, strtable, index);
                mapi_push_stringn(U, entry.string, entry.size);
            } else {
                mapi_push_nil(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void lex_create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            const char *text = mapi_get_string(U);
            size_t size = mapi_string_len(U);

            mcapi_push_lex(U, text, size);
            maux_nb_return();
    maux_nb_end
}

static void lex_token(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            struct mc_lex *lex = mcapi_get_lex(U);
            mapi_push_arg(U, 1);
            struct mc_strtable *strtable = mcapi_get_strtable(U);

            struct mc_lex_token token = mcapi_lex_step(U, lex, strtable);

            mapi_push_table(U);

            mapi_push_string(U, "type");
            switch (token.type) {
                case MCLTT_EOS:
                    mapi_push_string(U, "eos");
                    mapi_table_set(U);
                    break;
                case MCLTT_INTEGER:
                    mapi_push_string(U, "integer");
                    mapi_table_set(U);
                    break;
                case MCLTT_DECIMAL:
                    mapi_push_string(U, "decimal");
                    mapi_table_set(U);
                    break;
                case MCLTT_STRING:
                    mapi_push_string(U, "string");
                    mapi_table_set(U);
                    break;
                case MCLTT_OPERATOR:
                    mapi_push_string(U, "operator");
                    mapi_table_set(U);
                    break;
                case MCLTT_WORD:
                    mapi_push_string(U, "word");
                    mapi_table_set(U);
                    break;
                case MCLTT_EXTENDED_WORD:
                    mapi_push_string(U, "extended_word");
                    mapi_table_set(U);
                    break;
                case MCLTT_COMMENT:
                    mapi_push_string(U, "comment");
                    mapi_table_set(U);
                    break;
                case MCLTT_MULTILINE_COMMENT:
                    mapi_push_string(U, "multiline_comment");
                    mapi_table_set(U);
                    break;
            }

            mapi_push_string(U, "value");
            switch (token.type) {
                case MCLTT_EOS:
                    mapi_pop(U, 1);
                    break;
                case MCLTT_INTEGER:
                    mapi_push_integer(U, token.integer);
                    mapi_table_set(U);
                    break;
                case MCLTT_DECIMAL:
                    mapi_push_decimal(U, token.decimal);
                    mapi_table_set(U);
                    break;
                case MCLTT_STRING:
                    mapi_push_size(U, token.string, "index");
                    mapi_table_set(U);
                    break;
                case MCLTT_OPERATOR:
                    mapi_push_string(U, mcapi_lex_operator2name(U, token.op));
                    mapi_table_set(U);
                    break;
                case MCLTT_WORD:
                case MCLTT_EXTENDED_WORD:
                    mapi_push_size(U, token.word, "index");
                    mapi_table_set(U);
                    break;
                case MCLTT_COMMENT:
                case MCLTT_MULTILINE_COMMENT:
                    mapi_push_size(U, token.comment, "index");
                    mapi_table_set(U);
                    break;
            }

            mapi_push_string(U, "line");
            mapi_push_size(U, token.line, "line");
            mapi_table_set(U);

            mapi_push_string(U, "range");
            mapi_push_table(U);

            mapi_push_string(U, "from");
            mapi_push_size(U, token.range.from, "index");
            mapi_table_set(U);

            mapi_push_string(U, "to");
            mapi_push_size(U, token.range.to, "index");
            mapi_table_set(U);

            mapi_table_set(U);
            maux_nb_return();
    maux_nb_end
}

static void ast(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            bool yieldable = false;
            if (mapi_is_type(U, "table")) {
                mapi_push_string(U, "yieldable");
                if (mapi_table_get(U)) {
                    yieldable = mapi_get_boolean(U);
                }
                mapi_pop(U, 1);

                mapi_push_string(U, "text");
                mapi_table_get(U);
            }

            const char *text = mapi_get_string(U);
            ml_size text_len = mapi_string_len(U);

            if (!yieldable) {
                struct mc_strtable *T = mcapi_push_strtable(U);
                struct mc_ast *A = mcapi_push_ast(U);

                {
                    struct mc_lex *L = mcapi_push_lex(U, text, text_len);
                    struct mc_parser *P = mcapi_push_parser(U);
                    while (mcapi_parser_step(U, P, A, L, T)) { }
                    mapi_pop(U, 2);
                }

                {
                    struct mc_visitor *V = mcapi_push_visitor(U);
                    while (mcapi_serializer_step(U, V, T, A)) { }
                }

                maux_nb_return();
            } else {
                mapi_bind_registry(U);

                mcapi_push_strtable(U);
                maux_registry_set(U, "T");

                mcapi_push_ast(U);
                maux_registry_set(U, "A");

                mcapi_push_lex(U, text, text_len);
                maux_registry_set(U, "LV");

                mcapi_push_parser(U);
                maux_registry_set(U, "P");

                maux_nb_continue(1);
            }
        maux_nb_state(1)
            maux_registry_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_registry_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_registry_get(U, "LV");
            struct mc_lex *L = mcapi_get_lex(U);

            maux_registry_get(U, "P");
            struct mc_parser *P = mcapi_get_parser(U);

            bool next = mcapi_parser_step(U, P, A, L, T);
            mapi_pop(U, 4);

            if (next) {
                maux_nb_continue(1);
            } else {
                mcapi_push_visitor(U);
                maux_registry_set(U, "LV");

                maux_nb_continue(2);
            }
        maux_nb_state(2)
            maux_registry_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_registry_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_registry_get(U, "LV");
            struct mc_visitor *V = mcapi_get_visitor(U);

            mapi_pop(U, 3);

            if (mcapi_serializer_step(U, V, T, A)) {
                maux_nb_continue(2);
            }

            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "compile",         compile },
    { "disassembly",     disassembly },
    { "strtable.create", strtable_create },
    { "strtable.access", strtable_access },
    { "lex.create",      lex_create },
    { "lex.token",       lex_token },
    { "ast",             ast },

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
