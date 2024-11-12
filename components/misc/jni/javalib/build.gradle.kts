plugins {
    kotlin("jvm") version "2.0.20"
}

group = "ru.why"
version = "1.0"

repositories {
    mavenCentral()
}

kotlin {
    jvmToolchain(21)
}

tasks.jar {
    archiveFileName = "morphinejni.jar"

    if (project.hasProperty("jarOutputPath")) {
        destinationDirectory.set(File(project.property("jarOutputPath").toString()))
    }

    val contents = configurations.runtimeClasspath.get().map { classpath ->
        if (classpath.isDirectory) {
            classpath
        } else {
            zipTree(classpath)
        }
    }

    from(contents)
}
