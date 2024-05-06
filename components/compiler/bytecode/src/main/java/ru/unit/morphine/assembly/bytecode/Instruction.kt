package ru.unit.morphine.assembly.bytecode

import ru.unit.morphine.assembly.bytecode.annotation.*

sealed interface Instruction {

    val opcode: Opcode

    val orderedArguments: List<Argument>

    val description: String

    var lineData: LineData?

    @Control
    data class Yield(
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.YIELD

        override val orderedArguments = emptyList<Argument>()

        override val description = "yield"
    }

    @Producer
    data class Load(
        val constant: Argument.Constant,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.LOAD

        override val orderedArguments = listOf(constant, destination)

        override val description = "load $constant to $destination"
    }

    @Movement
    data class Move(
        val source: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.MOVE

        override val orderedArguments = listOf(source, destination)

        override val description = "move $source to $destination"
    }

    @Consumer
    data class Param(
        val source: Argument.Slot,
        val param: Argument.Param,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.PARAM

        override val orderedArguments = listOf(source, param)

        override val description = "move $source to $param"
    }

    @Producer
    data class Arg(
        val arg: Argument.Arg,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.ARG

        override val orderedArguments = listOf(arg, destination)

        override val description = "move $arg to $destination"
    }

    @Producer
    data class Environment(
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.ENV

        override val orderedArguments = listOf(destination)

        override val description = "move environment to $destination"
    }

    @Producer
    data class Self(
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.SELF

        override val orderedArguments = listOf(destination)

        override val description = "move self to $destination"
    }

    @Producer
    data class Recursion(
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.RECURSION

        override val orderedArguments = listOf(destination)

        override val description = "move callable to $destination"
    }

    @Producer
    data class Vector(
        val destination: Argument.Slot,
        val count: Argument.Count,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.VECTOR

        override val orderedArguments = listOf(destination, count)

        override val description = "create vector in $destination with size $count"
    }

    @Producer
    data class Table(
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.TABLE

        override val orderedArguments = listOf(destination)

        override val description = "create table in $destination"
    }

    @Processing
    data class Get(
        val container: Argument.Slot,
        val keySource: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.GET

        override val orderedArguments = listOf(container, keySource, destination)

        override val description = "get from $container by $keySource to $destination"
    }

    @Consumer
    data class Set(
        val source: Argument.Slot,
        val keySource: Argument.Slot,
        val container: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.SET

        override val orderedArguments = listOf(container, keySource, source)

        override val description = "set $source to $container by $keySource"
    }

    @Processing
    data class Iterator(
        val container: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.ITERATOR

        override val orderedArguments = listOf(container, destination)

        override val description = "create iterator from $container to $destination"
    }

    @Consumer
    data class IteratorInit(
        val iterator: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.ITERATOR_INIT

        override val orderedArguments = listOf(iterator)

        override val description = "init iterator $iterator"
    }

    @Processing
    data class IteratorHas(
        val iterator: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.ITERATOR_HAS

        override val orderedArguments = listOf(iterator, destination)

        override val description = "check next value of iterator $iterator to $destination"
    }

    @Processing
    data class IteratorNext(
        val iterator: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.ITERATOR_NEXT

        override val orderedArguments = listOf(iterator, destination)

        override val description = "get next value of iterator $iterator to $destination"
    }

    @Control
    data class Jump(
        val position: Argument.Position,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.JUMP

        override val orderedArguments = listOf(position)

        override val description = "move to $position"
    }

    @Consumer
    data class JumpIf(
        val source: Argument.Slot,
        val ifPosition: Argument.Position,
        val elsePosition: Argument.Position,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.JUMP_IF

        override val orderedArguments = listOf(source, ifPosition, elsePosition)

        override val description = "if $source is true move to $ifPosition else $elsePosition"
    }

    @Processing
    data class GetStatic(
        val callable: Argument.Slot,
        val index: Argument.Index,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.GET_STATIC

        override val orderedArguments = listOf(callable, index, destination)

        override val description = "get $index from $callable to $destination"
    }

    @Consumer
    data class SetStatic(
        val callable: Argument.Slot,
        val index: Argument.Index,
        val source: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.SET_STATIC

        override val orderedArguments = listOf(callable, index, source)

        override val description = "set $source to $index of $callable"
    }

    @Processing
    data class GetClosure(
        val closure: Argument.Slot,
        val index: Argument.Index,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.GET_CLOSURE

        override val orderedArguments = listOf(closure, index, destination)

        override val description = "get $index from $closure to $destination"
    }

    @Consumer
    data class SetClosure(
        val closure: Argument.Slot,
        val index: Argument.Index,
        val source: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.SET_CLOSURE

        override val orderedArguments = listOf(closure, index, source)

        override val description = "set $source to $index of $closure"
    }

    @Processing
    data class Closure(
        val source: Argument.Slot,
        val count: Argument.Count,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.CLOSURE

        override val orderedArguments = listOf(source, count, destination)

        override val description = "create closure from $source with size $count in $destination"
    }

    @Consumer
    data class SCall(
        val source: Argument.Slot,
        val selfSource: Argument.Slot,
        val count: Argument.Count,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.SCALL

        override val orderedArguments = listOf(source, count, selfSource)

        override val description = "call $source with $count args and self $selfSource"
    }

    @Consumer
    data class Call(
        val source: Argument.Slot,
        val count: Argument.Count,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.CALL

        override val orderedArguments = listOf(source, count)

        override val description = "call $source with $count args and self as nil"
    }

    @Consumer
    data class Leave(
        val source: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.LEAVE

        override val orderedArguments = listOf(source)

        override val description = "leave with return value from $source"
    }

    @Producer
    data class Result(
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.RESULT

        override val orderedArguments = listOf(destination)

        override val description = "set result to $destination"
    }

    @Processing
    data class Add(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.ADD

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft add $sourceRight to $destination"
    }

    @Processing
    data class Sub(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.SUB

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft sub $sourceRight to $destination"
    }

    @Processing
    data class Mul(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.MUL

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft mul $sourceRight to $destination"
    }

    @Processing
    data class Div(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.DIV

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft div $sourceRight to $destination"
    }

    @Processing
    data class Mod(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.MOD

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft mod $sourceRight to $destination"
    }

    @Processing
    data class Equal(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.EQUAL

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft equal $sourceRight to $destination"
    }

    @Processing
    data class Less(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.LESS

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft less $sourceRight to $destination"
    }

    @Processing
    data class LessEqual(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.LESS_EQUAL

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft less equal $sourceRight to $destination"
    }

    @Processing
    data class And(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.AND

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft and $sourceRight to $destination"
    }

    @Processing
    data class Or(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.OR

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft or $sourceRight to $destination"
    }

    @Processing
    data class Concat(
        val sourceLeft: Argument.Slot,
        val sourceRight: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.CONCAT

        override val orderedArguments = listOf(sourceLeft, sourceRight, destination)

        override val description = "$sourceLeft concat $sourceRight to $destination"
    }

    @Processing
    data class Type(
        val source: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.TYPE

        override val orderedArguments = listOf(source, destination)

        override val description = "get string type of $source to $destination"
    }

    @Processing
    data class Negative(
        val source: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.NEGATIVE

        override val orderedArguments = listOf(source, destination)

        override val description = "negative $source to $destination"
    }

    @Processing
    data class Not(
        val source: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.NOT

        override val orderedArguments = listOf(source, destination)

        override val description = "not $source to $destination"
    }

    @Processing
    data class Length(
        val source: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.LENGTH

        override val orderedArguments = listOf(source, destination)

        override val description = "length of $source to $destination"
    }

    @Processing
    data class Ref(
        val source: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.REF

        override val orderedArguments = listOf(source, destination)

        override val description = "ref of $source to $destination"
    }

    @Processing
    data class Deref(
        val source: Argument.Slot,
        val destination: Argument.Slot,
        override var lineData: LineData? = null
    ) : Instruction {
        override val opcode = Opcode.DEREF

        override val orderedArguments = listOf(source, destination)

        override val description = "deref of $source to $destination"
    }
}