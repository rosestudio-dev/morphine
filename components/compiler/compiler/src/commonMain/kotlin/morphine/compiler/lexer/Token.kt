package morphine.compiler.lexer

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
        RECURSIVE,
        FUN,
        IF,
        ELSE,
        ELIF,
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
        REF,
        END,
        PASS,
        ITERATOR,
        DECOMPOSE,
        AS,
        IN,
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
        DOTDOTEQ("..="),
        LARROW("<-"),
        RARROW("->");

        companion object {

            const val CHARS = "+-*/%()[]{}=<>!&|.,^~?:;"
        }
    }
}