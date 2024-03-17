import java.nio.file.Files
import java.nio.file.Paths

buildscript {
    repositories {
        gradlePluginPortal()
        google()
        mavenCentral()
    }

    dependencies {
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:1.9.23")
        classpath("com.google.devtools.ksp:com.google.devtools.ksp.gradle.plugin:1.9.23-1.0.19")
    }
}

group = "ru.unit.morphine"

subprojects {
    val customBuildDir = System.getProperty("customBuildDir")

    if (customBuildDir != null) {
        layout.buildDirectory.set(File(File(customBuildDir), project.fullname))
    }

    version = Files.readString(Paths.get(File(rootProject.projectDir.parentFile, "VERSION").path))
}

val Project.fullname
    get() = run {
        val projects = mutableListOf<Project>()

        var current: Project? = this
        while (current != null) {
            projects.add(current)
            current = current.parent
        }

        projects.reversed().joinToString(separator = "_", transform = Project::getName)
    }
