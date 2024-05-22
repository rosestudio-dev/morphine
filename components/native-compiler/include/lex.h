//
// Created by why-iskra on 19.05.2024.
//

#pragma once

#include <morphine.h>
#include "strtable.h"

enum token_type {
    TT_EOS,
    TT_INTEGER,
    TT_DECIMAL,
    TT_STRING,
    TT_WORD,
    TT_PREDEFINED_WORD,
    TT_OPERATOR,
};

enum token_operator {
    TOP_PLUS,       // +
    TOP_MINUS,      // -
    TOP_STAR,       // *
    TOP_SLASH,      // /
    TOP_PERCENT,    // %
    TOP_EQ,         // =
    TOP_EQEQ,       // ==
    TOP_EXCLEQ,     // !=
    TOP_LT,         // <
    TOP_GT,         // >
    TOP_LTEQ,       // <=
    TOP_GTEQ,       // >=
    TOP_COLON,      // :
    TOP_DOLLAR,     // $
    TOP_SEMICOLON,  // ;
    TOP_LPAREN,     // (
    TOP_RPAREN,     // )
    TOP_LBRACKET,   // [
    TOP_RBRACKET,   // ]
    TOP_LBRACE,     // {
    TOP_RBRACE,     // }
    TOP_COMMA,      // ,
    TOP_DOT,        // .
    TOP_DOTDOT,     // ..
    TOP_PLUSPLUS,   // ++
    TOP_MINUSMINUS, // --
    TOP_PLUSEQ,     // +=
    TOP_MINUSEQ,    // -=
    TOP_STAREQ,     // *=
    TOP_SLASHEQ,    // /=
    TOP_PERCENTEQ,  // %=
    TOP_DOTDOTEQ,   // ..=
    TOP_LARROW,     // <-
    TOP_RARROW,     // ->
};

enum token_predefined_word {
    TPW_true,
    TPW_false,
    TPW_env,
    TPW_self,
    TPW_nil,
    TPW_val,
    TPW_static,
    TPW_var,
    TPW_and,
    TPW_or,
    TPW_not,
    TPW_recursive,
    TPW_auto,
    TPW_fun,
    TPW_if,
    TPW_else,
    TPW_elif,
    TPW_while,
    TPW_do,
    TPW_for,
    TPW_break,
    TPW_continue,
    TPW_return,
    TPW_leave,
    TPW_eval,
    TPW_type,
    TPW_len,
    TPW_to,
    TPW_yield,
    TPW_ref,
    TPW_end,
    TPW_pass,
    TPW_iterator,
    TPW_decompose,
    TPW_as,
    TPW_in,
};

struct token {
    enum token_type type;
    uint32_t line;

    union {
        ml_integer integer;
        ml_decimal decimal;
        strtable_index_t string;
        strtable_index_t word;
        enum token_predefined_word predefined_word;
        enum token_operator op;
    };
};

void lex(morphine_coroutine_t, const char *, size_t);
struct token lex_next(morphine_coroutine_t);

const char *lex_op2str(enum token_operator);
const char *lex_predefined2str(enum token_predefined_word);
