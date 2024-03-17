plugins {
    id("org.gradle.toolchains.foojay-resolver-convention") version "0.5.0"
}
rootProject.name = "compiler"
include("cli:compiler")
include("compiler")
include("bytecode")
include("optimizer")
include("bytecode-processor")
include("bytecode-annotation")
