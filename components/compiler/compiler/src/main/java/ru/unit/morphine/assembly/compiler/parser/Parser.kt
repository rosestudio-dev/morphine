package ru.unit.morphine.assembly.compiler.parser

import ru.unit.morphine.assembly.bytecode.LineData
import ru.unit.morphine.assembly.bytecode.Value
import ru.unit.morphine.assembly.compiler.ast.Ast
import ru.unit.morphine.assembly.compiler.ast.node.AccessAccessible
import ru.unit.morphine.assembly.compiler.ast.node.Accessible
import ru.unit.morphine.assembly.compiler.ast.node.AssigmentStatement
import ru.unit.morphine.assembly.compiler.ast.node.AssignMethod
import ru.unit.morphine.assembly.compiler.ast.node.BinaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.BlockStatement
import ru.unit.morphine.assembly.compiler.ast.node.BreakStatement
import ru.unit.morphine.assembly.compiler.ast.node.CallExpression
import ru.unit.morphine.assembly.compiler.ast.node.CallSelfExpression
import ru.unit.morphine.assembly.compiler.ast.node.ContinueStatement
import ru.unit.morphine.assembly.compiler.ast.node.DeclarationStatement
import ru.unit.morphine.assembly.compiler.ast.node.DoWhileStatement
import ru.unit.morphine.assembly.compiler.ast.node.EmptyStatement
import ru.unit.morphine.assembly.compiler.ast.node.EnvExpression
import ru.unit.morphine.assembly.compiler.ast.node.EvalStatement
import ru.unit.morphine.assembly.compiler.ast.node.Expression
import ru.unit.morphine.assembly.compiler.ast.node.ForStatement
import ru.unit.morphine.assembly.compiler.ast.node.FunctionExpression
import ru.unit.morphine.assembly.compiler.ast.node.IfStatement
import ru.unit.morphine.assembly.compiler.ast.node.IncDecExpression
import ru.unit.morphine.assembly.compiler.ast.node.IteratorStatement
import ru.unit.morphine.assembly.compiler.ast.node.Node
import ru.unit.morphine.assembly.compiler.ast.node.ReturnStatement
import ru.unit.morphine.assembly.compiler.ast.node.SelfExpression
import ru.unit.morphine.assembly.compiler.ast.node.Statement
import ru.unit.morphine.assembly.compiler.ast.node.TableExpression
import ru.unit.morphine.assembly.compiler.ast.node.UnaryExpression
import ru.unit.morphine.assembly.compiler.ast.node.ValueExpression
import ru.unit.morphine.assembly.compiler.ast.node.VariableAccessible
import ru.unit.morphine.assembly.compiler.ast.node.VectorExpression
import ru.unit.morphine.assembly.compiler.ast.node.WhileStatement
import ru.unit.morphine.assembly.compiler.ast.node.YieldStatement
import ru.unit.morphine.assembly.compiler.lexer.Token
import ru.unit.morphine.assembly.compiler.parser.exception.ParseException

class Parser(
    private val tokens: List<Pair<Token, LineData>>,
    private val debug: Boolean
) {
    private var position = 0
    private var errors = mutableListOf<Pair<Int, Throwable>>()

    fun parse(): Ast {
        val list = mutableListOf<Statement>()

        while (!match(Token.Eof)) {
            val currentLine = data(position).lineData.line

            runCatching {
                list.add(statement())
            }.onFailure { throwable ->
                if (debug) {
                    throw throwable
                }

                errors.add(currentLine to throwable)
                recover()
            }
        }

        if (errors.isNotEmpty()) {
            val message = buildString {
                errors.forEachIndexed { index, pair ->
                    if (errors.size > 1) {
                        append("${index + 1}. ")
                    }

                    append("Line ${pair.first}: ")

                    if (pair.second !is ParseException) {
                        append("(${pair.second::class.simpleName}) ")
                    }

                    append(pair.second.message)

                    if (errors.size > 1) {
                        append("\n")
                    }
                }
            }

            throw ParseException(message)
        }

        val statement = BlockStatement(
            statements = list,
            data = data(-position)
        )

        return Ast(statement)
    }

    private fun recover() {
        val preRecoverPosition = position
        for (i in preRecoverPosition until tokens.size) {
            position = i

            val isSuccess = runCatching {
                statement()
                position = i
            }.isSuccess

            if (isSuccess) {
                break
            }
        }
    }

    private fun statement(allowSemicolon: Boolean = true) = when {
        look(Token.SystemWord.YIELD) -> statementYield()
        look(Token.SystemWord.IF) -> statementIf()
        look(Token.SystemWord.ITERATOR) -> statementIterator()
        look(Token.SystemWord.WHILE) -> statementWhile()
        look(Token.SystemWord.DO) -> statementDoWhile()
        look(Token.SystemWord.FOR) -> statementFor()
        look(Token.SystemWord.BREAK) -> statementBreak()
        look(Token.SystemWord.CONTINUE) -> statementContinue()
        look(Token.SystemWord.RETURN) -> statementReturn()
        look(Token.SystemWord.LEAVE) -> statementLeave()
        look(Token.SystemWord.EVAL) -> statementEval()
        look(Token.SystemWord.PASS) -> statementEmpty()
        look(Token.SystemWord.FUN) -> statementFun()
        look(Token.SystemWord.VAL, Token.SystemWord.VAR) -> statementDeclaration()
        else -> statementAssigment()
    }.also {
        if (allowSemicolon) {
            match(Token.Operator.SEMICOLON)
        }
    }

    private fun statementFun(): Statement {
        val saved = position

        val function = expressionFunction(requireName = true)

        return DeclarationStatement(
            method = AssignMethod.Single(function.name!!),
            isMutable = false,
            expression = function,
            data = data(saved)
        )
    }

    private fun statementBreak(): Statement {
        val saved = position
        consume(Token.SystemWord.BREAK)
        return BreakStatement(data(saved))
    }

    private fun statementContinue(): Statement {
        val saved = position
        consume(Token.SystemWord.CONTINUE)
        return ContinueStatement(data(saved))
    }

    private fun statementReturn(): Statement {
        val saved = position
        consume(Token.SystemWord.RETURN)

        return templateReturn(saved)
    }

    private fun statementLeave(): Statement {
        val saved = position
        consume(Token.SystemWord.LEAVE)

        return ReturnStatement(
            expression = ValueExpression(Value.Nil, data(saved)),
            data = data(saved)
        )
    }

    private fun statementEval(): Statement {
        val saved = position
        consume(Token.SystemWord.EVAL)
        return EvalStatement(expression(), data(saved))
    }

    private fun statementEmpty(): Statement {
        val saved = position
        consume(Token.SystemWord.PASS)
        return EmptyStatement(data(saved))
    }

    private fun statementBlock(
        vararg closes: Token,
        allowSingleStatement: Boolean = true
    ): BlockStatement {
        val saved = position

        return if (match(Token.Operator.COLON) && allowSingleStatement) {
            BlockStatement(listOf(statement()), data(saved))
        } else {
            val list = mutableListOf<Statement>()

            while (!look(*closes) && !match(Token.SystemWord.END)) {
                list.add(statement())
            }

            BlockStatement(list, data(saved))
        }
    }

    private fun statementYield(): YieldStatement {
        val saved = position

        consume(Token.SystemWord.YIELD)

        return YieldStatement(data(saved))
    }

    private fun statementIf(elif: Boolean = false): IfStatement {
        val saved = position

        if (elif) {
            consume(Token.SystemWord.ELIF)
        } else {
            consume(Token.SystemWord.IF)
        }

        consume(Token.Operator.LPAREN)
        val condition = expression()
        consume(Token.Operator.RPAREN)

        val ifStatement = statementBlock(Token.SystemWord.ELSE, Token.SystemWord.ELIF)

        val elseStatement = when {
            look(Token.SystemWord.ELIF) -> statementIf(elif = true)
            match(Token.SystemWord.ELSE) -> statementBlock()
            else -> EmptyStatement(data(saved))
        }

        return IfStatement(
            condition = condition,
            ifStatement = ifStatement,
            elseStatement = elseStatement,
            data(saved)
        )
    }

    private fun statementFor(): Statement {
        val saved = position

        consume(Token.SystemWord.FOR)

        consume(Token.Operator.LPAREN)

        val initial = statement(allowSemicolon = false)
        consume(Token.Operator.SEMICOLON)
        val condition = expression()
        consume(Token.Operator.SEMICOLON)
        val iterator = statement(allowSemicolon = false)

        consume(Token.Operator.RPAREN)

        val statement = statementBlock()

        return ForStatement(
            initial = initial,
            condition = condition,
            iterator = iterator,
            statement = statement,
            data = data(saved)
        )
    }

    private fun statementWhile(): Statement {
        val saved = position

        consume(Token.SystemWord.WHILE)

        consume(Token.Operator.LPAREN)
        val condition = expression()
        consume(Token.Operator.RPAREN)

        val statement = statementBlock()

        return WhileStatement(
            condition = condition,
            statement = statement,
            data = data(saved)
        )
    }

    private fun statementIterator(): Statement {
        val saved = position

        consume(Token.SystemWord.ITERATOR)

        consume(Token.Operator.LPAREN)
        val method = assignMethod {
            consumeWord().text
        }

        consume(Token.SystemWord.IN)

        val expression = expression()

        consume(Token.Operator.RPAREN)

        val statement = statementBlock()

        return IteratorStatement(
            method = method,
            iterable = expression,
            statement = statement,
            data = data(saved)
        )
    }

    private fun statementDoWhile(): Statement {
        val saved = position

        consume(Token.SystemWord.DO)

        val statement = statementBlock()

        if (match(Token.SystemWord.WHILE)) {
            consume(Token.Operator.LPAREN)
            val condition = expression()
            consume(Token.Operator.RPAREN)

            return DoWhileStatement(
                condition = condition,
                statement = statement,
                data = data(saved)
            )
        } else {
            return statement
        }
    }

    private fun statementDeclaration(): Statement {
        val saved = position

        val isMutable = when {
            match(Token.SystemWord.VAL) -> false
            match(Token.SystemWord.VAR) -> true
            else -> throw ParseException("Expected ${Token.SystemWord.VAL} or ${Token.SystemWord.VAR}, but got ${get()}")
        }

        val method = assignMethod { consumeWord().text }

        consume(Token.Operator.EQ)

        val expression = expression()

        return DeclarationStatement(
            method = method,
            isMutable = isMutable,
            expression = expression,
            data = data(saved)
        )
    }

    private fun statementAssigment(): Statement {
        val saved = position

        val method = assignMethod { expression() }

        return when (method) {
            is AssignMethod.Decompose -> {
                consume(Token.Operator.EQ)
                val expression = expression()

                val entries = method.entries.map { entry ->
                    val accessible = entry.key as? Accessible
                        ?: throw ParseException("Expression isn't accessible")

                    AssignMethod.Decompose.Entry(
                        value = accessible,
                        key = entry.key
                    )
                }

                AssigmentStatement(
                    method = AssignMethod.Decompose(entries),
                    expression = expression,
                    binaryType = null,
                    data = data(saved)
                )
            }

            is AssignMethod.Single -> if (look(*ASSIGN_OPERATORS.keys.toTypedArray())) {
                val type = get()
                skip()

                val expression = expression()

                val entry = method.entry as? Accessible ?: throw ParseException("Expression isn't accessible")

                AssigmentStatement(
                    method = AssignMethod.Single(entry),
                    expression = expression,
                    binaryType = ASSIGN_OPERATORS[type],
                    data = data(saved)
                )
            } else {
                val expression = method.entry

                EvalStatement(
                    expression = expression,
                    data = data(saved)
                )
            }
        }
    }

    private fun expression(): Expression {
        return expressionConcat()
    }

    private fun expressionConcat(): Expression {
        var result = expressionOr()

        while (true) {
            if (match(Token.Operator.DOTDOT)) {
                val saved = position - 1
                result = BinaryExpression(
                    type = BinaryExpression.Type.CONCAT,
                    expressionA = result,
                    expressionB = expressionOr(),
                    data = data(saved)
                )
            } else {
                break
            }
        }

        return result
    }

    private fun expressionOr(): Expression {
        val result = expressionAnd()

        return if (match(Token.SystemWord.OR)) {
            val saved = position - 1
            BinaryExpression(
                type = BinaryExpression.Type.OR,
                expressionA = result,
                expressionB = expressionOr(),
                data = data(saved)
            )
        } else {
            result
        }
    }

    private fun expressionAnd(): Expression {
        val result = expressionEqual()

        return if (match(Token.SystemWord.AND)) {
            val saved = position - 1
            BinaryExpression(
                type = BinaryExpression.Type.AND,
                expressionA = result,
                expressionB = expressionAnd(),
                data = data(saved)
            )
        } else {
            result
        }
    }

    private fun expressionEqual(): Expression {
        var result = expressionConditional()

        while (true) {
            when {
                match(Token.Operator.EQEQ) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.EQUALS,
                        expressionA = result,
                        expressionB = expressionConditional(),
                        data = data(saved)
                    )
                }

                match(Token.Operator.EXCLEQ) -> {
                    val saved = position - 1
                    result = UnaryExpression(
                        type = UnaryExpression.Type.NOT,
                        expression = BinaryExpression(
                            type = BinaryExpression.Type.EQUALS,
                            expressionA = result,
                            expressionB = expressionConditional(),
                            data = data(saved)
                        ),
                        data = data(saved)
                    )
                }

                else -> break
            }
        }

        return result
    }

    private fun expressionConditional(): Expression {
        var result = expressionAdditive()

        while (true) {
            when {
                match(Token.Operator.LT) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.LESS,
                        expressionA = result,
                        expressionB = expressionAdditive(),
                        data = data(saved)
                    )
                }

                match(Token.Operator.GT) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.LESS,
                        expressionA = expressionAdditive(),
                        expressionB = result,
                        data = data(saved)
                    )
                }

                match(Token.Operator.LTEQ) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.LESS_EQUALS,
                        expressionA = result,
                        expressionB = expressionAdditive(),
                        data = data(saved)
                    )
                }

                match(Token.Operator.GTEQ) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.LESS_EQUALS,
                        expressionA = expressionAdditive(),
                        expressionB = result,
                        data = data(saved)
                    )
                }

                else -> break
            }
        }

        return result
    }

    private fun expressionAdditive(): Expression {
        var result = expressionMultiplicative()

        while (true) {
            when {
                match(Token.Operator.PLUS) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.ADD,
                        expressionA = result,
                        expressionB = expressionMultiplicative(),
                        data = data(saved)
                    )
                }

                match(Token.Operator.MINUS) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.SUB,
                        expressionA = result,
                        expressionB = expressionMultiplicative(),
                        data = data(saved)
                    )
                }

                else -> break
            }
        }

        return result
    }

    private fun expressionMultiplicative(): Expression {
        var result = expressionPrefix()

        while (true) {
            when {
                match(Token.Operator.STAR) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.MUL,
                        expressionA = result,
                        expressionB = expressionPrefix(),
                        data = data(saved)
                    )
                }

                match(Token.Operator.SLASH) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.DIV,
                        expressionA = result,
                        expressionB = expressionPrefix(),
                        data = data(saved)
                    )
                }

                match(Token.Operator.PERCENT) -> {
                    val saved = position - 1
                    result = BinaryExpression(
                        type = BinaryExpression.Type.MOD,
                        expressionA = result,
                        expressionB = expressionPrefix(),
                        data = data(saved)
                    )
                }

                else -> break
            }
        }

        return result
    }

    private fun expressionPrefix(): Expression {
        val saved = position

        return when {
            match(Token.SystemWord.NOT) -> UnaryExpression(
                type = UnaryExpression.Type.NOT,
                expression = expressionPrefix(),
                data = data(saved)
            )

            match(Token.Operator.MINUS) -> UnaryExpression(
                type = UnaryExpression.Type.NEGATE,
                expression = expressionPrefix(),
                data = data(saved)
            )

            match(Token.SystemWord.TYPE) -> UnaryExpression(
                type = UnaryExpression.Type.TYPE,
                expression = expressionPrefix(),
                data = data(saved)
            )

            match(Token.SystemWord.LEN) -> UnaryExpression(
                type = UnaryExpression.Type.LEN,
                expression = expressionPrefix(),
                data = data(saved)
            )

            match(Token.SystemWord.REF) -> UnaryExpression(
                type = UnaryExpression.Type.REF,
                expression = expressionPrefix(),
                data = data(saved)
            )

            match(Token.Operator.STAR) -> UnaryExpression(
                type = UnaryExpression.Type.DEREF,
                expression = expressionPrefix(),
                data = data(saved)
            )

            match(Token.Operator.PLUSPLUS) -> IncDecExpression(
                type = IncDecExpression.Type.INCREMENT,
                isPostfix = false,
                accessible = expressionPrefix() as? Accessible
                    ?: throw ParseException("Increment requires accessible expression"),
                data = data(saved)
            )

            match(Token.Operator.MINUSMINUS) -> IncDecExpression(
                type = IncDecExpression.Type.DECREMENT,
                isPostfix = false,
                accessible = expressionPrefix() as? Accessible
                    ?: throw ParseException("Decrement requires accessible expression"),
                data = data(saved)
            )

            else -> expressionPostfix()
        }
    }

    private fun expressionPostfix(): Expression {
        val expression = expressionChain()

        return when {
            match(Token.Operator.PLUSPLUS) -> IncDecExpression(
                type = IncDecExpression.Type.INCREMENT,
                isPostfix = true,
                accessible = expression as? Accessible
                    ?: throw ParseException("Increment requires accessible expression"),
                data = data(position - 1)
            )

            match(Token.Operator.MINUSMINUS) -> IncDecExpression(
                type = IncDecExpression.Type.DECREMENT,
                isPostfix = true,
                accessible = expression as? Accessible
                    ?: throw ParseException("Decrement requires accessible expression"),
                data = data(position - 1)
            )

            else -> expression
        }
    }

    private fun expressionChain(): Expression {
        var result = expressionValue()

        while (true) {
            when {
                match(Token.Operator.LBRACKET) -> {
                    val saved = position - 1
                    result = AccessAccessible(
                        container = result,
                        key = expression(),
                        data = data(saved)
                    )

                    consume(Token.Operator.RBRACKET)
                }

                match(Token.Operator.DOT) -> {
                    val saved = position - 1

                    val text = consumeWord().text
                    result = AccessAccessible(
                        container = result,
                        key = ValueExpression(Value.String(text), data(saved + 1)),
                        data = data(saved)
                    )
                }

                look(Token.Operator.LPAREN) -> {
                    val saved = position - 1

                    val arguments = arguments(
                        determinator = Token.Operator.COMMA,
                        open = Token.Operator.LPAREN,
                        close = Token.Operator.RPAREN
                    ) { expression() }

                    result = CallExpression(
                        expression = result,
                        arguments = arguments,
                        data = data(saved)
                    )
                }

                look(Token.Operator.LBRACE) -> {
                    val saved = position - 1

                    val table = expressionTable()

                    result = CallExpression(
                        expression = result,
                        arguments = listOf(table),
                        data = data(saved)
                    )
                }

                match(Token.Operator.COLON) -> {
                    val saved = position - 1

                    val callable = when {
                        match(Token.Operator.LBRACKET) -> {
                            expression().also {
                                consume(Token.Operator.RBRACKET)
                            }
                        }

                        else -> {
                            val text = consumeWord().text

                            ValueExpression(
                                value = Value.String(text),
                                data = data(saved + 1)
                            )
                        }
                    }

                    val arguments = if (look(Token.Operator.LBRACE)) {
                        listOf(expressionTable())
                    } else {
                        arguments(
                            determinator = Token.Operator.COMMA,
                            open = Token.Operator.LPAREN,
                            close = Token.Operator.RPAREN
                        ) { expression() }
                    }

                    result = CallSelfExpression(
                        self = result,
                        callable = callable,
                        arguments = arguments,
                        extractCallable = true,
                        data = data(saved)
                    )
                }

                match(Token.Operator.RARROW) -> {
                    val saved = position - 1

                    val callable = when {
                        match(Token.Operator.LBRACKET) -> {
                            expression().also {
                                consume(Token.Operator.RBRACKET)
                            }
                        }

                        else -> {
                            val text = consumeWord().text

                            VariableAccessible(
                                name = text,
                                data = data(saved + 1)
                            )
                        }
                    }

                    val arguments = if (look(Token.Operator.LBRACE)) {
                        listOf(expressionTable())
                    } else {
                        arguments(
                            determinator = Token.Operator.COMMA,
                            open = Token.Operator.LPAREN,
                            close = Token.Operator.RPAREN
                        ) { expression() }
                    }

                    result = CallSelfExpression(
                        self = result,
                        callable = callable,
                        extractCallable = false,
                        arguments = arguments,
                        data = data(saved)
                    )
                }

                else -> break
            }
        }

        return result
    }

    private fun expressionValue(): Expression = when {
        look(Token.SystemWord.NIL) -> expressionNil()
        lookNumber() -> expressionNumber()
        lookText() -> expressionText()
        look(Token.SystemWord.TRUE) -> expressionTrue()
        look(Token.SystemWord.FALSE) -> expressionFalse()
        lookWord() -> expressionVariable()
        look(Token.SystemWord.ENV) -> expressionEnv()
        look(Token.SystemWord.SELF) -> expressionSelf()
        look(Token.SystemWord.FUN) -> expressionFunction()
        look(Token.Operator.LBRACE) -> expressionTable()
        look(Token.Operator.LBRACKET) -> expressionVector()
        look(Token.SystemWord.IF) -> statementIf()
        look(Token.SystemWord.DO) -> expressionBlock()
        match(Token.Operator.LPAREN) -> expression().also { consume(Token.Operator.RPAREN) }
        else -> throw ParseException("Unknown expression: ${get()}")
    }

    private fun expressionBlock(): BlockStatement {
        val saved = position

        consume(Token.SystemWord.DO)

        val list = mutableListOf<Statement>()

        while (!match(Token.SystemWord.END)) {
            list.add(statement())
        }

        return BlockStatement(list, data(saved))
    }

    private fun expressionNil() = ValueExpression(
        value = Value.Nil,
        data = data(position)
    ).also { consume(Token.SystemWord.NIL) }

    private fun expressionNumber() = ValueExpression(
        value = (get() as Token.Number).number.createNumber(),
        data = data(position)
    ).also { consumeNumber() }

    private fun expressionText() = ValueExpression(
        value = Value.String((get() as Token.Text).text),
        data = data(position)
    ).also { consumeText() }

    private fun expressionTrue() = ValueExpression(
        value = Value.Boolean(true),
        data = data(position)
    ).also { consume(Token.SystemWord.TRUE) }

    private fun expressionFalse() = ValueExpression(
        value = Value.Boolean(false),
        data = data(position)
    ).also { consume(Token.SystemWord.FALSE) }

    private fun expressionVariable() = VariableAccessible(
        name = (get() as Token.Word).text,
        data = data(position)
    ).also { consumeWord() }

    private fun expressionEnv() = EnvExpression(
        data = data(position)
    ).also { consume(Token.SystemWord.ENV) }

    private fun expressionSelf() = SelfExpression(
        data = data(position)
    ).also { consume(Token.SystemWord.SELF) }

    private fun expressionTable(): Expression {
        val saved = position

        var index = 0

        val elements = arguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LBRACE,
            close = Token.Operator.RBRACE
        ) {
            val expression = expression()

            val element = if (match(Token.SystemWord.TO)) {
                TableExpression.Element(
                    key = expression,
                    value = expression()
                )
            } else {
                TableExpression.Element(
                    key = ValueExpression(
                        value = Value.Integer(index),
                        data = data(saved)
                    ),
                    value = expression
                )
            }

            index++

            element
        }

        return TableExpression(
            elements = elements,
            data = data(saved)
        )
    }

    private fun expressionVector(): Expression {
        val saved = position

        val elements = arguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LBRACKET,
            close = Token.Operator.RBRACKET
        ) {
            expression()
        }

        return VectorExpression(
            elements = elements,
            data = data(saved)
        )
    }

    private fun expressionFunction(
        requireName: Boolean = false
    ): FunctionExpression {
        val saved = position

        consume(Token.SystemWord.FUN)

        val name = if (lookWord() || requireName) {
            consumeWord().text
        } else {
            null
        }

        val arguments = arguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LPAREN,
            close = Token.Operator.RPAREN
        ) {
            consumeWord().text
        }

        val statics = if (match(Token.SystemWord.STATIC)) {
            arguments(
                determinator = Token.Operator.COMMA,
                open = Token.Operator.LPAREN,
                close = Token.Operator.RPAREN
            ) {
                consumeWord().text
            }
        } else {
            emptyList()
        }

        val statement = if (match(Token.Operator.EQ)) {
            templateReturn(
                saved = position - 1,
                allowMultiple = false
            )
        } else {
            statementBlock()
        }

        return FunctionExpression(
            name = name,
            statics = statics,
            arguments = arguments,
            statement = statement,
            data = data(saved)
        )
    }

    private fun <T> arguments(
        determinator: Token,
        open: Token? = null,
        close: Token? = null,
        consumeOpen: Boolean = true,
        consumeClose: Boolean = true,
        parse: () -> T
    ): List<T> {
        open?.let {
            if (consumeOpen) {
                consume(open)
            } else {
                match(open)
            }
        }

        if (close != null && match(close)) {
            return emptyList()
        }

        val result = mutableListOf(parse())

        while (match(determinator)) {
            if (close != null && match(close)) {
                return result
            }

            result.add(parse())
        }

        close?.let {
            if (consumeClose) {
                consume(close)
            } else {
                match(close)
            }
        }

        return result
    }

    private fun <T> assignMethod(
        parse: () -> T
    ): AssignMethod<T> = if (match(Token.SystemWord.DECOMPOSE)) {
        val values = arguments(
            determinator = Token.Operator.COMMA,
            open = Token.Operator.LPAREN,
            close = Token.Operator.RPAREN,
            consumeOpen = false,
            consumeClose = false,
            parse = parse
        ).ifEmpty {
            throw ParseException("Empty decompose")
        }

        val saved = position

        val keys = if (match(Token.SystemWord.AS)) {
            arguments(
                determinator = Token.Operator.COMMA,
                open = Token.Operator.LPAREN,
                close = Token.Operator.RPAREN,
                consumeOpen = false,
                consumeClose = false,
            ) { expression() }
        } else {
            List(values.size) { index ->
                ValueExpression(
                    value = Value.Integer(index),
                    data = data(saved)
                )
            }
        }

        val entries = values.mapIndexed { index, value ->
            val key = keys.getOrNull(index) ?: throw ParseException(
                "Key for ${index + 1} decompose entry wasn't defined"
            )

            AssignMethod.Decompose.Entry(
                value = value,
                key = key
            )
        }

        AssignMethod.Decompose(entries)
    } else {
        AssignMethod.Single(
            entry = parse()
        )
    }

    private fun templateReturn(
        saved: Int,
        allowMultiple: Boolean = true
    ): ReturnStatement {
        val expressions = mutableListOf(expression())
        while (allowMultiple && match(Token.Operator.COMMA)) {
            expressions.add(expression())
        }

        val single = expressions.singleOrNull()
        return if (single == null) {
            val elements = expressions.mapIndexed { index, expression ->
                TableExpression.Element(
                    key = ValueExpression(Value.Integer(index), data(saved)),
                    value = expression
                )
            }

            val table = TableExpression(elements, data(saved))
            ReturnStatement(table, data(saved))
        } else {
            ReturnStatement(single, data(saved))
        }
    }

    private fun String.createNumber() = if (contains(".")) {
        Value.Decimal(toDouble())
    } else {
        Value.Integer(toInt())
    }

    private fun skip() {
        position++
    }

    private fun consume(token: Token): Token {
        val current = get()

        return if (current == token) {
            position++
            current
        } else {
            throw ParseException("Token $current doesn't match $token")
        }
    }

    private fun match(token: Token) = (get() == token).also { result ->
        if (result) {
            position++
        }
    }

    private fun look(
        vararg tokens: Token,
        relative: Int = 0
    ) = tokens.any { token -> get(relative) == token }

    private fun get(
        relative: Int = 0
    ) = tokens.getOrNull(position + relative)?.first ?: Token.Eof

    private fun data(saved: Int) = Node.Data(
        lineData = tokens.getOrNull(saved)?.second
            ?: tokens.lastOrNull()?.second
            ?: LineData(line = 0, column = 0)
    )

    private fun consumeWord(): Token.Word {
        val current = get()

        return if (current is Token.Word) {
            position++
            current
        } else {
            throw ParseException("Token $current doesn't match Word")
        }
    }

    private fun lookWord() = get() is Token.Word

    private fun consumeText(): Token.Text {
        val current = get()

        return if (current is Token.Text) {
            position++
            current
        } else {
            throw ParseException("Token $current doesn't match Text")
        }
    }

    private fun lookText() = get() is Token.Text

    private fun consumeNumber(): Token.Number {
        val current = get()

        return if (current is Token.Number) {
            position++
            current
        } else {
            throw ParseException("Token $current doesn't match Number")
        }
    }

    private fun lookNumber() = get() is Token.Number

    companion object {

        private val ASSIGN_OPERATORS = mapOf(
            Token.Operator.EQ to null,
            Token.Operator.PLUSEQ to BinaryExpression.Type.ADD,
            Token.Operator.MINUSEQ to BinaryExpression.Type.SUB,
            Token.Operator.STAREQ to BinaryExpression.Type.MUL,
            Token.Operator.SLASHEQ to BinaryExpression.Type.DIV,
            Token.Operator.PERCENTEQ to BinaryExpression.Type.MOD,
            Token.Operator.DOTDOTEQ to BinaryExpression.Type.CONCAT,
        )
    }
}