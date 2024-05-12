package morphine.utils

import kotlin.time.TimeSource

interface UID {
    val mostBits: Long
    val leastBits: Long
}

class CommonUID : UID {
    private val time = TimeSource.Monotonic
        .markNow()
        .elapsedNow()
        .inWholeMicroseconds

    override val mostBits = time
    override val leastBits = random()

    private fun random(): Long {
        val array = secureRandom(8)
        var result = 0L

        repeat(8) { index ->
            result = result.or(array[index].toLong() shl (index * 8))
        }

        return result
    }
}

fun UID() = CommonUID()
