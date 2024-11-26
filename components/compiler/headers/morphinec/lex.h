//
// Created by why-iskra on 19.05.2024.
//

#pragma once

#include <morphine.h>
#include "morphinec/strtable.h"

#define MC_LEX_USERDATA_TYPE "morphinec-lexer"

enum mc_lex_token_type {
    MCLTT_EOS,
    MCLTT_INTEGER,
    MCLTT_DECIMAL,
    MCLTT_STRING,
    MCLTT_OPERATOR,
    MCLTT_WORD,
    MCLTT_EXTENDED_WORD,
    MCLTT_COMMENT,
    MCLTT_MULTILINE_COMMENT,
};

enum mc_lex_token_operator {
    MCLTOP_PLUS,       // +
    MCLTOP_MINUS,      // -
    MCLTOP_STAR,       // *
    MCLTOP_SLASH,      // /
    MCLTOP_PERCENT,    // %
    MCLTOP_EQ,         // =
    MCLTOP_EQEQ,       // ==
    MCLTOP_EXCLEQ,     // !=
    MCLTOP_LT,         // <
    MCLTOP_GT,         // >
    MCLTOP_LTEQ,       // <=
    MCLTOP_GTEQ,       // >=
    MCLTOP_COLON,      // :
    MCLTOP_SEMICOLON,  // ;
    MCLTOP_LPAREN,     // (
    MCLTOP_RPAREN,     // )
    MCLTOP_LBRACKET,   // [
    MCLTOP_RBRACKET,   // ]
    MCLTOP_LBRACE,     // {
    MCLTOP_RBRACE,     // }
    MCLTOP_COMMA,      // ,
    MCLTOP_DOT,        // .
    MCLTOP_DOTDOT,     // ..
    MCLTOP_PLUSPLUS,   // ++
    MCLTOP_MINUSMINUS, // --
    MCLTOP_PLUSEQ,     // +=
    MCLTOP_MINUSEQ,    // -=
    MCLTOP_STAREQ,     // *=
    MCLTOP_SLASHEQ,    // /=
    MCLTOP_PERCENTEQ,  // %=
    MCLTOP_DOTDOTEQ,   // ..=
    MCLTOP_LARROW,     // <-
    MCLTOP_RARROW,     // ->
    MCLTOP_EXCL,       // !
};

struct mc_lex_token {
    enum mc_lex_token_type type;
    ml_line line;
    ml_size index;

    struct {
        ml_size from;
        ml_size to;
    } range;

    union {
        ml_integer integer;
        ml_decimal decimal;
        mc_strtable_index_t string;
        mc_strtable_index_t word;
        mc_strtable_index_t comment;
        enum mc_lex_token_operator op;
    };
};

struct mc_lex;

MORPHINE_API struct mc_lex *mcapi_push_lex(morphine_coroutine_t);
MORPHINE_API struct mc_lex *mcapi_get_lex(morphine_coroutine_t);

MORPHINE_API bool mcapi_lex_is_end(struct mc_lex *);
MORPHINE_API struct mc_lex_token mcapi_lex_step(morphine_coroutine_t, struct mc_lex *, struct mc_strtable *);

MORPHINE_API const char *mcapi_lex_type2str(morphine_coroutine_t, enum mc_lex_token_type);
MORPHINE_API const char *mcapi_lex_operator2str(morphine_coroutine_t, enum mc_lex_token_operator);
MORPHINE_API const char *mcapi_lex_operator2name(morphine_coroutine_t, enum mc_lex_token_operator);
