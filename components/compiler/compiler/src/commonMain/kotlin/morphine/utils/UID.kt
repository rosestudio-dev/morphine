package morphine.utils

import kotlin.random.Random

interface UID {
    val mostBits: Long
    val leastBits: Long
}

data class CommonUID(
    override val mostBits: Long,
    override val leastBits: Long
) : UID {

    private val string by lazy {
        val randomMost = Random(mostBits)
        val randomLeast = Random(leastBits)
        val randomShuffle = Random(mostBits.xor(leastBits))

        val comp1 = (0 until LEN / 2).map { toStringChars[randomMost.nextInt(toStringChars.size)] }
        val comp2 = (0 until LEN / 2).map { toStringChars[randomLeast.nextInt(toStringChars.size)] }

        (comp1 + comp2).shuffled(randomShuffle).joinToString(separator = "")
    }

    override fun toString() = string

    companion object {

        private const val LEN = 12
        private val toStringChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789".toCharArray()

        fun random(): CommonUID {
            val array = secureRandom(8)
            var rand = 0L

            repeat(8) { index ->
                rand = rand.or(array[index].toLong().and(0xFF) shl (index * 8))
            }

            val time = millis()

            return CommonUID(
                mostBits = time,
                leastBits = rand
            )
        }
    }
}

fun UID() = CommonUID.random()
