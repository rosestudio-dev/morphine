package ru.unit.morphine.assembly.compiler.lexer

sealed interface Token {

    data object Eof : Token

    data class Number(
        val number: String,
    ) : Token

    data class Text(
        val text: String,
    ) : Token

    data class Word(
        val text: String,
    ) : Token

    enum class SystemWord : Token {
        TRUE,
        FALSE,
        ENV,
        SELF,
        NIL,
        VAL,
        STATIC,
        VAR,
        AND,
        OR,
        NOT,
        FUN,
        IF,
        ELSE,
        WHILE,
        DO,
        FOR,
        BREAK,
        CONTINUE,
        RETURN,
        LEAVE,
        EVAL,
        TYPE,
        LEN,
        TO,
        YIELD,
        ASM,
        CLOSURES,
        REF,
        END,
        PASS
    }

    enum class Operator(val text: String) : Token {
        PLUS("+"),
        MINUS("-"),
        STAR("*"),
        SLASH("/"),
        PERCENT("%"),
        EQ("="),
        EQEQ("=="),
        EXCLEQ("!="),
        LTEQ("<="),
        LT("<"),
        GT(">"),
        GTEQ(">="),
        COLON(":"),
        DOLLAR("$"),
        SEMICOLON(";"),
        LPAREN("("),
        RPAREN(")"),
        LBRACKET("["),
        RBRACKET("]"),
        LBRACE("{"),
        RBRACE("}"),
        COMMA(","),
        DOT("."),
        DOTDOT(".."),
        PLUSPLUS("++"),
        MINUSMINUS("--"),
        PLUSEQ("+="),
        MINUSEQ("-="),
        STAREQ("*="),
        SLASHEQ("/="),
        PERCENTEQ("%="),
        DOTDOTEQ("..=");

        companion object {

            const val CHARS = "+-*/%()[]{}=<>!&|.,^~?:;"
        }
    }
}