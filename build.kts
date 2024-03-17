import java.io.File
import java.nio.file.Files
import java.nio.file.Path
import kotlin.system.exitProcess

val version = Files.readString(Path.of("VERSION"))

println("Welcome to Morphine builder!")
println("Version: $version")
println()

// file structure

val vmDir = File("virtual-machine")
val compilerDir = File("compiler")
val buildDir = File("build")

val buildCacheDir = File(buildDir, ".cache")
val buildIncludeDir = File(buildDir, "include")
val buildBinDir = File(buildDir, "bin")
val buildLibDir = File(buildDir, "lib")
val buildStaticLibDir = File(buildLibDir, "static")
val buildDynamicLibDir = File(buildLibDir, "dynamic")
val buildJarDir = File(buildDir, "jar")

// artifacts

val artifactBuildCacheCompilerDir = File(buildCacheDir, "compiler")
val artifactBuildMorphineCompiler = File(
    artifactBuildCacheCompilerDir,
    "compiler_cli_compiler/libs/morphine-compiler-${version}.jar"
)
val artifactLibMorphineCompiler = File(buildJarDir, "morphine-compiler.jar")
val artifactBinMorphineCompiler = File(buildBinDir, "morphine-compiler")

val artifactBuildCacheVmDir = File(buildCacheDir, "vm")
val artifactBuildMorphineVmApp = File(
    artifactBuildCacheVmDir,
    "application/morphine-app"
)
val artifactBuildMorphineVmStaticLib = File(
    artifactBuildCacheVmDir,
    "morphine/libmorphine.a"
)
val artifactBuildMorphineDebugVmStaticLib = File(
    artifactBuildCacheVmDir,
    "morphine/libmorphine-debug.a"
)
val artifactStaticLibMorphineVm = File(buildStaticLibDir, "libmorphine.a")
val artifactStaticLibMorphineDebugVm = File(buildStaticLibDir, "libmorphinedebug.a")
val artifactBinMorphineVm = File(buildBinDir, "morphine")

// commands

enum class Command(val argName: String) {
    BUILD("build"),
    CLEAN("clean"),
}

enum class Target(val argName: String) {
    COMPILER("compiler"),
    VIRTUAL_MACHINE("vm"),
    ALL("all"),
}

// software

data class Software(
    val cc: String,
    val make: String,
    val cmake: String,
    val gradle: String,
    val nativeimage: String,
    val java: String,
)

// main

val software = software()
run()

// functions

fun run() {
    if (args.isEmpty()) {
        build(Target.ALL)
    } else {
        val command = when (args.getOrNull(0)) {
            Command.BUILD.argName -> Command.BUILD
            Command.CLEAN.argName -> Command.CLEAN
            else -> {
                System.err.println("Wrong command")
                exitProcess(1)
            }
        }

        val target = when (args.getOrNull(1)) {
            Target.COMPILER.argName -> Target.COMPILER
            Target.VIRTUAL_MACHINE.argName -> Target.VIRTUAL_MACHINE
            else -> Target.ALL
        }

        when(command) {
            Command.BUILD -> build(target)
            Command.CLEAN -> clean(target)
        }
    }
}

fun software(): Software {
    println("Check software:")
    val paths = System.getenv("PATH").split(":")

    fun find(program: String): String? {
        print("    '$program' ")
        val result = paths.find { path ->
            File(File(path), program).exists()
        }

        if (result != null) {
            println("Exists")
        } else {
            println("Not found")
        }

        return result?.let { path -> File(File(path), program).toString() }
    }

    val cc = find("cc")
    val make = find("make")
    val cmake = find("cmake")
    val gradle = find("gradle")
    val nativeimage = find("native-image")
    val java = find("java")

    println()

    return Software(
        cc = cc ?: exitProcess(1),
        make = make ?: exitProcess(1),
        cmake = cmake ?: exitProcess(1),
        gradle = gradle ?: exitProcess(1),
        nativeimage = nativeimage ?: exitProcess(1),
        java = java ?: exitProcess(1),
    )
}

fun prepare() {
    buildDir.mkdir()
    buildCacheDir.mkdir()
    buildIncludeDir.mkdir()
    buildBinDir.mkdir()
    buildLibDir.mkdir()
    buildStaticLibDir.mkdir()
    buildDynamicLibDir.mkdir()
    buildJarDir.mkdir()
}

fun clean(target: Target) {
    when (target) {
        Target.COMPILER -> {
            artifactBuildCacheCompilerDir.deleteRecursively()

            artifactLibMorphineCompiler.delete()
            artifactBinMorphineCompiler.delete()
        }

        Target.VIRTUAL_MACHINE -> {
            artifactBuildCacheVmDir.deleteRecursively()

            artifactBinMorphineVm.delete()
            artifactStaticLibMorphineVm.delete()
            artifactStaticLibMorphineDebugVm.delete()
        }

        Target.ALL -> {
            buildDir.deleteRecursively()
        }
    }
}

fun build(target: Target) {
    prepare()

    when (target) {
        Target.COMPILER -> buildCompiler()
        Target.VIRTUAL_MACHINE -> buildVm()
        Target.ALL -> {
            buildCompiler()
            buildVm()
        }
    }
}

fun buildVm() {
    println("Build virtual-machine")

    artifactBinMorphineVm.delete()
    artifactStaticLibMorphineVm.delete()
    artifactStaticLibMorphineDebugVm.delete()

    ProcessBuilder(
        software.cmake,
        "-B",
        artifactBuildCacheVmDir.absolutePath,
        "-DCMAKE_BUILD_TYPE=Release"
    ).directory(vmDir)
        .redirectOutput(ProcessBuilder.Redirect.INHERIT)
        .redirectError(ProcessBuilder.Redirect.INHERIT)
        .start()
        .waitFor()
        .checkExitCode("build virtual-machine")

    ProcessBuilder(
        software.make
    ).directory(artifactBuildCacheVmDir)
        .redirectOutput(ProcessBuilder.Redirect.INHERIT)
        .redirectError(ProcessBuilder.Redirect.INHERIT)
        .start()
        .waitFor()
        .checkExitCode("build virtual-machine")

    artifactBuildMorphineVmApp.copyTo(
        target = artifactBinMorphineVm,
        overwrite = true
    )

    artifactBuildMorphineVmStaticLib.copyTo(
        target = artifactStaticLibMorphineVm,
        overwrite = true
    )

    artifactBuildMorphineDebugVmStaticLib.copyTo(
        target = artifactStaticLibMorphineDebugVm,
        overwrite = true
    )

    println()
}

fun buildCompiler() {
    println("Build compiler")

    artifactLibMorphineCompiler.delete()
    artifactBinMorphineCompiler.delete()

    ProcessBuilder(
        software.gradle,
        "jar",
        "-DcustomBuildDir=${artifactBuildCacheCompilerDir.absolutePath}"
    ).directory(compilerDir)
        .redirectOutput(ProcessBuilder.Redirect.INHERIT)
        .redirectError(ProcessBuilder.Redirect.INHERIT)
        .start()
        .waitFor()
        .checkExitCode("build compiler")

    artifactBuildMorphineCompiler.copyTo(
        target = artifactLibMorphineCompiler,
        overwrite = true
    )

    ProcessBuilder(
        software.nativeimage,
        "--no-server",
        "--no-fallback",
        "-jar",
        artifactLibMorphineCompiler.absolutePath,
        artifactBinMorphineCompiler.absolutePath
    ).directory(compilerDir)
        .redirectOutput(ProcessBuilder.Redirect.INHERIT)
        .redirectError(ProcessBuilder.Redirect.INHERIT)
        .start()
        .waitFor()
        .checkExitCode("build compiler")

    println()
}

fun Int.checkExitCode(message: String) {
    if (this != 0) {
        System.err.println("Errors during $message")
        exitProcess(1)
    }
}
