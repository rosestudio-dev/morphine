package ru.unit.morphine.assembly.bytecode

import java.nio.ByteBuffer
import java.util.UUID

class BytecodeConverter {

    private companion object {
        const val POLY: UInt = 0xEDB88320U
        const val INITIAL: UInt = 0xFFFFFFFFU
        const val XOROUT: UInt = 0xFFFFFFFFU

        const val FORMAT_TAG = "morphine-bout"
    }

    fun convert(bytecode: Bytecode) = (header() + bytecode(bytecode)).crc32Modifier()

    private fun header() = bytesBuilder(FORMAT_TAG.length) {
        put(FORMAT_TAG.toByteArray())
    }

    private fun bytecode(bytecode: Bytecode): List<Byte> {
        val uuid = bytecode.mainFunction.convert()
        val size = bytecode.functions.size.convert()
        val functions = bytecode.functions.flatMap(::function)
        val instructions = bytecode.functions.flatMap(::instructions)
        val lines = bytecode.functions.flatMap(::lines)
        val constants = bytecode.functions.flatMap(::constants)
        val names = bytecode.functions.flatMap { function ->
            function.name.convert(needLen = false)
        }

        return uuid + size + functions + instructions + lines + constants + names
    }

    private fun function(function: Bytecode.Function) = listOf(
        function.uuid.convert(),
        function.name.length.convertAsShort("Name of function too long"),
        function.argumentsCount.convertAsShort("Count of arguments too big"),
        function.slotsCount.convertAsShort("Count of slots too big"),
        function.paramsCount.convertAsShort("Count of params too big"),
        function.staticsCount.convertAsShort("Count of statics too big"),
        function.constants.size.convertAsShort("Count of constants too big"),
        function.instructions.size.convertAsShort("Count of instructions too big"),
    ).flatten()

    private fun instructions(function: Bytecode.Function) =
        function.instructions.flatMap { instruction ->
            val opcode = instruction.opcode.ordinal.toByte().convert()
            val arguments = instruction.orderedArguments.flatMap { argument ->
                argument.value.convertAsShort("Argument value too big")
            }

            opcode + arguments
        }

    private fun lines(function: Bytecode.Function) =
        function.instructions.flatMap { instruction ->
            (instruction.lineData?.line ?: 0).convert()
        }

    private fun constants(
        function: Bytecode.Function
    ) = function.constants.flatMap { constant ->
        when (constant) {
            is Value.Boolean -> listOf(
                'b'.convert(),
                listOf(
                    if (constant.value) {
                        1.toByte()
                    } else {
                        0.toByte()
                    }
                )
            )

            is Value.Integer -> listOf(
                'i'.convert(),
                constant.value.convert()
            )

            is Value.Function -> listOf(
                'f'.convert(),
                constant.value.convert()
            )

            is Value.Decimal -> listOf(
                'd'.convert(),
                constant.value.convert()
            )

            is Value.String -> listOf(
                's'.convert(),
                constant.value.convert(needLen = true)
            )

            Value.Nil -> listOf(
                'n'.convert()
            )
        }
    }.flatten()

    private fun UUID.convert() = bytesBuilder(16) {
        putLong(this@convert.mostSignificantBits)
        putLong(this@convert.leastSignificantBits)
    }

    private fun Double.convert() = bytesBuilder(8) {
        putDouble(this@convert)
    }

    private fun Int.convertAsShort(error: String? = null) = bytesBuilder(2) {
        if (error != null && this@convertAsShort > UShort.MAX_VALUE.toInt()) {
            throw ConvertException(error)
        } else {
            putShort(this@convertAsShort.toShort())
        }
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
            else -> throw ConvertException("Int len too big")
        }
    }

    private fun String.convert(needLen: Boolean) = this@convert.toByteArray().let { bytes ->
        val len = if (needLen) {
            bytes.size.convert().toByteArray()
        } else {
            null
        }

        bytesBuilder((len?.size ?: 0) + bytes.size) {
            len?.let { put(len) }
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

    class ConvertException(override val message: String) : Exception()
}
