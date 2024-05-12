rootProject.name = "compiler"

include("cli")
include("library")
include("compiler")
include("annotation")
include("processor")

pluginManagement {
    repositories {
        gradlePluginPortal()
        mavenCentral()
        google()
    }

    val kotlinVersion: String by settings
    val kspVersion: String by settings
    val buildconfigVersion: String by settings

    plugins {
        kotlin("multiplatform").version(kotlinVersion)
        id("com.google.devtools.ksp").version(kspVersion)
        id("com.github.gmazzo.buildconfig").version(buildconfigVersion)
    }
}

plugins {
    id("org.gradle.toolchains.foojay-resolver-convention") version "0.5.0"
}

dependencyResolutionManagement {
    repositories {
        mavenCentral()
        google()
    }
}