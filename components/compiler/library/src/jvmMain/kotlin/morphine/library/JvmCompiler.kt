package morphine.library

import compiler.library.BuildConfig
import morphine.Compiler

object JvmCompiler {

    fun instance(
        text: String,
        optimize: Boolean
    ) = Compiler(
        text = text,
        optimize = optimize
    )

    fun versionCode() = BuildConfig.versionCode

    fun version() = BuildConfig.version
}
