import kotlin.reflect.KProperty

object Properties {

    val outputDir by SystemProperty("")
    val monoBuildDir by SystemProperty("")
    val projectVersion by SystemProperty("unknown")
    val projectVersionCode by SystemProperty("0")
}

private class SystemProperty(private val default: String) {
    operator fun getValue(thisRef: Any?, property: KProperty<*>): String =
        System.getProperty(property.name) ?: default
}
