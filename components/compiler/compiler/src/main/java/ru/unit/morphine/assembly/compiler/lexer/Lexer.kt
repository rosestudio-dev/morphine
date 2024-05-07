package ru.unit.morphine.assembly.compiler.lexer

import ru.unit.morphine.assembly.bytecode.LineData
import ru.unit.morphine.assembly.compiler.lexer.exception.LexerException

class Lexer(private val text: String) {

    fun tokenize(): List<LinedToken> {
        val tokens = mutableListOf<LinedToken>()
        val carriage = Carriage(text)

        carriage.update(shift = false)

        do {
            val current = carriage.peek(0)

            carriage.save()

            val token = when {
                current.isEof() -> LinedToken(token = Token.Eof, lineData = carriage.lineData())
                Character.isDigit(current) -> tokenizeNumber(carriage)
                Character.isLetter(current) || current == '_' -> tokenizeWord(carriage)
                current == '#' -> tokenizeHexNumber(carriage)
                current == '`' -> tokenizeExtendedWord(carriage)
                current == '"' || current == '\'' -> tokenizeText(carriage, current)
                current == '/' && carriage.peek(1) == '/' -> tokenizeComment(carriage)
                current == '/' && carriage.peek(1) == '*' -> tokenizeMultilineComment(carriage)
                Token.Operator.CHARS.indexOf(current) != -1 -> tokenizeOperator(carriage)
                else -> tokenizeNext(carriage)
            } ?: continue

            tokens.add(token)
        } while (!current.isEof())

        return tokens
    }

    private fun tokenizeNext(carriage: Carriage): LinedToken? {
        carriage.next()
        return null
    }

    private fun tokenizeNumber(carriage: Carriage): LinedToken {
        var current = carriage.peek(0)

        val buffer = buildString {
            while (true) {
                when {
                    current.isDigit() || current == '.' -> append(current)
                    current == '_' -> Unit
                    else -> {
                        break
                    }
                }

                current = carriage.next()
            }
        }

        if (buffer.count { it == '.' } > 1) {
            throw carriage.error("Invalid decimal number")
        }

        return LinedToken(
            token = Token.Number(buffer),
            lineData = carriage.lineData()
        )
    }

    private fun tokenizeHexNumber(carriage: Carriage): LinedToken {
        carriage.next()

        var current = carriage.peek(0)

        val buffer = buildString {
            while (true) {
                when {
                    current.isHex() -> append(current)
                    current == '_' -> continue
                    else -> break
                }

                current = carriage.next()
            }
        }

        return runCatching {
            LinedToken(
                token = Token.Number(buffer.toInt(16).toString()),
                lineData = carriage.lineData()
            )
        }.getOrElse {
            throw carriage.error("Invalid hex number")
        }
    }

    private fun tokenizeText(carriage: Carriage, startChar: Char): LinedToken {
        var current = carriage.next()

        val buffer = buildString {
            while (true) {
                if (current.isEof()) {
                    throw carriage.error("Reached end of file while parsing text")
                }

                if (current == startChar) {
                    break
                }

                if (current == '\\') {
                    current = carriage.next()

                    when (current) {
                        '\\' -> append('\\')
                        '"' -> append('"')
                        '\'' -> append('\'')
                        '0' -> append('\u0000')
                        'b' -> append('\b')
                        'n' -> append('\n')
                        'r' -> append('\r')
                        't' -> append('\t')
                        else -> throw carriage.error("Unknown escape symbol")
                    }

                    current = carriage.next()
                    continue
                }

                append(current)

                current = carriage.next()
            }
        }

        carriage.next()

        return LinedToken(
            token = Token.Text(buffer),
            lineData = carriage.lineData()
        )
    }

    private fun tokenizeWord(carriage: Carriage): LinedToken {
        var current = carriage.peek(0)

        val buffer = buildString {
            while (true) {
                if (current.isLetterOrDigit() || current == '_') {
                    append(current)
                } else {
                    break
                }

                current = carriage.next()
            }
        }

        return LinedToken(
            token = SYSTEM_WORDS[buffer] ?: Token.Word(buffer),
            lineData = carriage.lineData()
        )
    }

    private fun tokenizeExtendedWord(carriage: Carriage): LinedToken {
        carriage.next() // skip `

        var current = carriage.peek(0)

        val buffer = buildString {
            while (true) {
                if (current.isEof()) {
                    throw carriage.error("Reached end of file while parsing extended word")
                } else if (current.isNewLine()) {
                    throw carriage.error("Reached end of line while parsing extended word")
                } else if (current == '`') {
                    break
                } else {
                    append(current)
                }

                current = carriage.next()
            }
        }

        carriage.next() // skip `

        return LinedToken(
            token = SYSTEM_WORDS[buffer] ?: Token.Word(buffer),
            lineData = carriage.lineData()
        )
    }

    private fun tokenizeOperator(carriage: Carriage): LinedToken {
        var current = carriage.peek(0)

        val buffer = buildString {
            append(current)

            while (true) {
                val string = toString()
                val operator = (string + carriage.peek(1))
                val isOperator = Token.Operator.entries.any { type ->
                    type.text == operator
                }

                if (!isOperator) {
                    break
                } else {
                    append(carriage.peek(1))
                }

                current = carriage.next()
            }
        }

        carriage.next()

        val operator = Token.Operator.entries.singleOrNull { type ->
            type.text == buffer
        } ?: throw carriage.error("Unknown operator")

        return LinedToken(
            token = operator,
            lineData = carriage.lineData()
        )
    }

    private fun tokenizeComment(carriage: Carriage): LinedToken? {
        carriage.next() // skip /
        carriage.next() // skip /

        var current = carriage.peek(0)

        while (!(current.isNewLine() || current.isEof())) {
            current = carriage.next()
        }

        return null
    }

    private fun tokenizeMultilineComment(carriage: Carriage): LinedToken? {
        carriage.next() // skip /
        carriage.next() // skip *

        var level = 1
        var current = carriage.peek(0)

        while (level > 0) {
            if (current.isEof()) {
                throw carriage.error("Reached end of line while parsing multiline comment")
            } else if (current == '/' && carriage.peek(1) == '*') {
                carriage.next()
                level++
            } else if (current == '*' && carriage.peek(1) == '/') {
                carriage.next()
                level--
            }

            current = carriage.next()
        }

        return null
    }

    private fun Char.isHex() = this.isDigit() || this.lowercaseChar() in 'a'..'f'

    private fun Char.isEof() = this == '\u0000'

    private fun Char.isNewLine() = this == '\n' || this == '\r'

    private class Carriage(text: String) {
        private var text = (text + '\u0000')

        private var state = State()
        private var saved = State()

        fun save() {
            saved = state.copy()
        }

        fun next(): Char {
            state.position++
            update()

            return peek(0)
        }

        fun peek(relative: Int): Char {
            val newPos = state.position + relative

            return if (newPos >= text.length) {
                '\u0000'
            } else {
                text[newPos]
            }
        }

        fun lineData() = LineData(
            line = saved.line,
            column = saved.column,
        )

        fun error(message: String) = LexerException(
            text = message,
            lineData = LineData(
                line = saved.line,
                column = saved.column,
            ),
        )

        fun update(shift: Boolean = true) {
            when (peek(0)) {
                '\n' -> {
                    state.line++
                    state.column = 0
                }

                else -> if (shift) {
                    state.column++
                }
            }
        }

        data class State(
            var position: Int = 0,
            var line: Int = 1,
            var column: Int = 1
        )
    }

    private companion object {

        val SYSTEM_WORDS = Token.SystemWord.entries.associateBy { type -> type.name.lowercase() }
    }
}