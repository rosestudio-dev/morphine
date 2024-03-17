plugins {
    application
    kotlin("jvm")
}

group = "ru.unit.morphine.assembly.cli.compiler"

repositories {
    mavenCentral()
}

application {
    mainClass.set("ru.unit.morphine.assembly.cli.compiler.MainKt")
}

tasks.jar {
    manifest.attributes["Main-Class"] = application.mainClass

    val dependencies = configurations
        .runtimeClasspath
        .get()
        .map(::zipTree)

    from(dependencies)
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE

    archiveBaseName.set("morphine-compiler")
}

dependencies {
    implementation(project(":bytecode"))
    implementation(project(":compiler"))
    implementation(kotlin("stdlib"))
    implementation("org.jetbrains.kotlinx:kotlinx-cli:0.3.4")
}