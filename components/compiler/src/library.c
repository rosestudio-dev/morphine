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
                mcapi_push_lex(U);
                maux_localstorage_set(U, "LV");

                mapi_push_boolean(U, vector);
                maux_localstorage_set(U, "vector");

                mapi_push_string(U, name);
                maux_localstorage_set(U, "name");

                mcapi_push_strtable(U);
                maux_localstorage_set(U, "T");

                mcapi_push_ast(U);
                maux_localstorage_set(U, "A");

                mcapi_push_parser(U);
                maux_localstorage_set(U, "PG");

                maux_nb_continue(1);
            }
        maux_nb_state(1)
            maux_localstorage_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_localstorage_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_localstorage_get(U, "LV");
            struct mc_lex *L = mcapi_get_lex(U);

            maux_localstorage_get(U, "PG");
            struct mc_parser *P = mcapi_get_parser(U);

            bool next = mcapi_parser_step(U, P, A, L, T);
            mapi_pop(U, 4);

            if (next) {
                maux_nb_continue(1);
            } else {
                mcapi_push_visitor(U);
                maux_localstorage_set(U, "LV");

                mcapi_push_codegen(U);
                maux_localstorage_set(U, "PG");

                maux_nb_continue(2);
            }
        maux_nb_state(2)
            maux_localstorage_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_localstorage_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_localstorage_get(U, "LV");
            struct mc_visitor *V = mcapi_get_visitor(U);

            maux_localstorage_get(U, "PG");
            struct mc_codegen *G = mcapi_get_codegen(U);

            if (mcapi_codegen_step(U, G, V, T, A)) {
                mapi_pop(U, 4);
                maux_nb_continue(2);
            }

            maux_localstorage_get(U, "vector");
            bool vector = mapi_get_boolean(U);

            maux_localstorage_get(U, "name");
            const char *name = mapi_get_cstr(U);

            mcapi_codegen_build(U, G, T, A, name, vector);
            maux_nb_return();
    maux_nb_end
}

static void dis(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            bool stream_io = false;
            if (mapi_args(U) == 2) {
                stream_io = true;
            } else {
                maux_expect_args(U, 1);
            }

            mapi_push_table(U);

            mapi_push_arg(U, stream_io ? 1 : 0);
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

                if (stream_io) {
                    mapi_push_arg(U, 0);
                } else {
                    mapi_push_stream_io(U);
                }

                mapi_rotate(U, 2);

                mcapi_disassembly(U);

                if (index != mapi_table_len(U) - 1) {
                    if (stream_io) {
                        mapi_push_arg(U, 0);
                    } else {
                        mapi_push_stream_io(U);
                    }

                    mapi_stream_print(U, "\n");
                    mapi_pop(U, 1);
                }
            }
            maux_nb_leave();
    maux_nb_end
}

static void push_token(
    morphine_coroutine_t U,
    struct mc_strtable *T,
    struct mc_lex_token token
) {
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
        case MCLTT_OPERATOR:
            mapi_push_string(U, mcapi_lex_operator2name(U, token.op));
            mapi_table_set(U);
            break;
        case MCLTT_STRING: {
            struct mc_strtable_entry entry = mcapi_strtable_access(U, T, token.string);
            mapi_push_stringn(U, entry.string, entry.size);
            mapi_table_set(U);
            break;
        }
        case MCLTT_WORD:
        case MCLTT_EXTENDED_WORD: {
            struct mc_strtable_entry entry = mcapi_strtable_access(U, T, token.word);
            mapi_push_stringn(U, entry.string, entry.size);
            mapi_table_set(U);
            break;
        }
        case MCLTT_COMMENT:
        case MCLTT_MULTILINE_COMMENT: {
            struct mc_strtable_entry entry = mcapi_strtable_access(U, T, token.comment);
            mapi_push_stringn(U, entry.string, entry.size);
            mapi_table_set(U);
            break;
        }
    }

    mapi_push_string(U, "line");
    mapi_push_size(U, token.line, "line");
    mapi_table_set(U);

    mapi_push_string(U, "index");
    mapi_push_size(U, token.index, "index");
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
}

static void lex(morphine_coroutine_t U) {
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

            if (!yieldable) {
                struct mc_lex *lex = mcapi_push_lex(U);
                struct mc_strtable *strtable = mcapi_push_strtable(U);

                mapi_push_vector(U, 0);
                mapi_vector_mode_fixed(U, false);
                struct mc_lex_token token;
                do {
                    token = mcapi_lex_step(U, lex, strtable);
                    push_token(U, strtable, token);
                    mapi_vector_push(U);
                } while (!mcapi_lex_is_end(lex));
                maux_nb_return();
            } else {
                mcapi_push_lex(U);
                maux_localstorage_set(U, "L");

                mcapi_push_strtable(U);
                maux_localstorage_set(U, "T");

                mapi_push_vector(U, 0);
                mapi_vector_mode_fixed(U, false);

                maux_nb_continue(1);
            }
        maux_nb_state(1)
            maux_localstorage_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_localstorage_get(U, "L");
            struct mc_lex *L = mcapi_get_lex(U);

            mapi_pop(U, 2);

            struct mc_lex_token token = mcapi_lex_step(U, L, T);
            push_token(U, T, token);
            mapi_vector_push(U);

            if (mcapi_lex_is_end(L)) {
                maux_nb_return();
            } else {
                maux_nb_continue(1);
            }
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

            if (!yieldable) {
                struct mc_strtable *T = mcapi_push_strtable(U);
                struct mc_ast *A = mcapi_push_ast(U);

                {
                    mapi_peek(U, 2);
                    struct mc_lex *L = mcapi_push_lex(U);
                    struct mc_parser *P = mcapi_push_parser(U);
                    while (mcapi_parser_step(U, P, A, L, T)) { }
                    mapi_pop(U, 3);
                }

                {
                    struct mc_visitor *V = mcapi_push_visitor(U);
                    while (mcapi_serializer_step(U, V, T, A)) { }
                }

                maux_nb_return();
            } else {
                mcapi_push_lex(U);
                maux_localstorage_set(U, "LV");

                mcapi_push_strtable(U);
                maux_localstorage_set(U, "T");

                mcapi_push_ast(U);
                maux_localstorage_set(U, "A");

                mcapi_push_parser(U);
                maux_localstorage_set(U, "P");

                maux_nb_continue(1);
            }
        maux_nb_state(1)
            maux_localstorage_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_localstorage_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_localstorage_get(U, "LV");
            struct mc_lex *L = mcapi_get_lex(U);

            maux_localstorage_get(U, "P");
            struct mc_parser *P = mcapi_get_parser(U);

            bool next = mcapi_parser_step(U, P, A, L, T);
            mapi_pop(U, 4);

            if (next) {
                maux_nb_continue(1);
            } else {
                mcapi_push_visitor(U);
                maux_localstorage_set(U, "LV");

                maux_nb_continue(2);
            }
        maux_nb_state(2)
            maux_localstorage_get(U, "T");
            struct mc_strtable *T = mcapi_get_strtable(U);

            maux_localstorage_get(U, "A");
            struct mc_ast *A = mcapi_get_ast(U);

            maux_localstorage_get(U, "LV");
            struct mc_visitor *V = mcapi_get_visitor(U);

            mapi_pop(U, 3);

            if (mcapi_serializer_step(U, V, T, A)) {
                maux_nb_continue(2);
            }

            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("compile", compile),
    MAUX_CONSTRUCT_FUNCTION("dis", dis),
    MAUX_CONSTRUCT_FUNCTION("lex", lex),
    MAUX_CONSTRUCT_FUNCTION("ast", ast),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mclib_compiler(void) {
    return (morphine_library_t) {
        .name = "compiler",
        .sharedkey = NULL,
        .init = library_init
    };
}
