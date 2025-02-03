plugins {
    kotlin("jvm") version "2.0.20"
}

group = "ru.why"
version = "1.0"

repositories {
    mavenCentral()
}

kotlin {
    jvmToolchain(8)
}

fun Configuration.toContent() = map { classpath ->
    if (classpath.isDirectory) {
        classpath
    } else {
        zipTree(classpath)
    }
}

tasks.jar {
    archiveFileName = "morphinejni.jar"

    if (project.hasProperty("jarOutputPath")) {
        destinationDirectory.set(File(project.property("jarOutputPath").toString()))
    }
}

task<Jar>("fatJar") {
    dependsOn(tasks.compileKotlin)

    archiveFileName = "morphinejni.jar"

    if (project.hasProperty("jarOutputPath")) {
        destinationDirectory.set(File(project.property("jarOutputPath").toString()))
    }

    val runtime = configurations.runtimeClasspath.get().toContent()
    val sources = sourceSets.main.get().runtimeClasspath.filter { it.isDirectory }

    from(runtime + sources)
}
