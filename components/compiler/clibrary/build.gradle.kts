plugins {
    kotlin("jvm")
}

group = "ru.unit.morphine.assembly.clibrary"

repositories {
    mavenCentral()
}

tasks.jar {
    manifest.attributes["Main-Class"] = ""
    manifest.attributes["Implementation-Version"] = version

    val dependencies = configurations
        .runtimeClasspath
        .get()
        .map(::zipTree)

    from(dependencies)
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE

    archiveBaseName.set("clib")

    System.getProperty("jarOutputDir")?.let { jarOutputDir ->
        val dir = File(jarOutputDir).apply { mkdir() }

        destinationDirectory.set(dir)
    }
}

dependencies {
    implementation(project(":bytecode"))
    implementation(project(":compiler"))
}