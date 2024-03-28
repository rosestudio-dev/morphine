package ru.unit.morphine.assembly.bytecode

import java.nio.ByteBuffer
import java.util.*

class BytecodeConverter {

    private companion object {
        const val POLY: UInt = 0xEDB88320U
        const val INITIAL: UInt = 0xFFFFFFFFU
        const val XOROUT: UInt = 0xFFFFFFFFU

        const val FORMAT_TAG = "morphine-bout"
    }

    fun convert(bytecode: Bytecode) = listOf(
        FORMAT_TAG.toByteArray().toList(),
        bytecode.mainFunction.convert(),
        bytecode.functions.convert()
    ).flatten().crc32Modifier()

    @OptIn(ExperimentalUnsignedTypes::class)
    private fun List<Byte>.crc32Modifier() = run {
        var crc = INITIAL
        val table = crc32Table()

        val array = this@crc32Modifier.toByteArray()

        array.forEach { byte ->
            crc = table[crc.xor(byte.toUInt().and(0x000000FFU)).toInt().and(0xFF)].xor(crc.shr(8))
        }

        val result = crc.xor(XOROUT)

        this@crc32Modifier + result.toInt().convert()
    }

    private fun List<Bytecode.Function>.convert(): List<Byte> = listOf(
        size.convert(),
        flatMap { function ->
            function.convert()
        },
        flatMap { function ->
            function.convertConstants()
        },
        flatMap { function ->
            function.convertName()
        }
    ).flatten()

    private fun Bytecode.Function.convert(): List<Byte> = listOf(
        uuid.convert(),
        name.length.convert(),
        argumentsCount.convert(),
        slotsCount.convert(),
        paramsCount.convert(),
        closuresCount.convert(),
        staticsCount.convert(),
        constants.size.convert(),
        instructions.size.convert(),
        instructions.flatMap { instruction ->
            instruction.convert()
        }
    ).flatten()

    private fun Instruction.convert() = listOf(
        (lineData?.line ?: 0).convert(),
        opcode.ordinal.toByte().convert(),
        orderedArguments.size.toByte().convert(),
        orderedArguments.flatMap { argument ->
            argument.convert()
        }
    ).flatten()

    private fun Argument.convert() = when (this) {
        is Argument.Constant -> 'c'.convert()
        is Argument.Slot -> 's'.convert()
        is Argument.Count -> 'o'.convert()
        is Argument.Arg -> 'a'.convert()
        is Argument.Param -> 'p'.convert()
        is Argument.Position -> 'i'.convert()
        is Argument.Closure -> 'l'.convert()
        is Argument.Static -> 't'.convert()
    } + value.convert()

    private fun Bytecode.Function.convertName(): List<Byte> = name.map { value ->
        value.convert()
    }.flatten()

    private fun Bytecode.Function.convertConstants(): List<Byte> = constants.map { value ->
        value.convert()
    }.flatten()

    private fun Value.convert() = when (this) {
        is Value.Boolean -> listOf(
            'b'.convert(),
            listOf(
                if (value) {
                    1.toByte()
                } else {
                    0.toByte()
                }
            )
        )

        is Value.Integer -> listOf(
            'i'.convert(),
            value.convert()
        )

        is Value.Function -> listOf(
            'f'.convert(),
            value.convert()
        )

        is Value.Decimal -> listOf(
            'd'.convert(),
            value.convert()
        )

        is Value.String -> listOf(
            's'.convert(),
            value.convert()
        )

        Value.Nil -> listOf(
            'n'.convert()
        )
    }.flatten()

    private fun UUID.convert() = bytesBuilder(16) {
        putLong(this@convert.mostSignificantBits)
        putLong(this@convert.leastSignificantBits)
    }

    private fun Double.convert() = bytesBuilder(8) {
        putDouble(this@convert)
    }

    private fun Int.convert(): List<Byte> {
        val bytes = bytesBuilder(4) {
            putInt(this@convert)
        }

        var countZero = 0
        for (b in bytes) {
            if (b != 0.toByte()) {
                break
            }

            countZero++
        }

        return when (countZero) {
            0 -> bytesBuilder(5) {
                put('i'.convert().toByteArray())
                put(bytes.toByteArray())
            }

            1 -> bytesBuilder(4) {
                put('h'.convert().toByteArray())
                put(bytes[1])
                put(bytes[2])
                put(bytes[3])
            }

            2 -> bytesBuilder(3) {
                put('s'.convert().toByteArray())
                put(bytes[2])
                put(bytes[3])
            }

            3 -> 'b'.convert() + bytes[3]
            4 -> 'z'.convert()
            else -> throw ConvertException()
        }
    }

    private fun String.convert() = this@convert.toByteArray().let { bytes ->
        val len = bytes.size.convert().toByteArray()

        bytesBuilder(len.size + bytes.size) {
            put(len)
            put(bytes)
        }
    }

    private fun Char.convert() = bytesBuilder(1) {
        put(this@convert.code.toByte())
    }

    private fun Byte.convert() = bytesBuilder(1) {
        put(this@convert)
    }

    private fun bytesBuilder(
        size: Int,
        body: ByteBuffer.() -> Unit
    ) = ByteBuffer.allocate(size).apply {
        body()
    }.array().toList()

    @OptIn(ExperimentalUnsignedTypes::class)
    private fun crc32Table(): UIntArray {
        val table = UIntArray(256)

        repeat(256) { index ->
            var gen = index.toUInt()

            repeat(8) {
                gen = if (gen.and(0x1U) == 0x1U) {
                    gen.shr(1).xor(POLY)
                } else {
                    gen.shr(1)
                }
            }

            table[index] = gen
        }

        return table
    }

    @OptIn(ExperimentalUnsignedTypes::class)
    private fun crc32PrintCConstants() {
        println("static const uint32_t POLY = 0x${POLY.toString(16)};")
        println("static const uint32_t INITIAL = 0x${INITIAL.toString(16)};")
        println("static const uint32_t XOROUT = 0x${XOROUT.toString(16)};")
        println("static const uint32_t TABLE[256] = { ${crc32Table().joinToString { "0x${it.toString(16)}" }} };")
    }

    class ConvertException : Exception()
}