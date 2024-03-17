plugins {
    kotlin("jvm")
}

group = "ru.unit.morphine.assembly.optimizer"

repositories {
    mavenCentral()
}

dependencies {
    implementation(project(":bytecode"))
    implementation(kotlin("stdlib"))
    implementation("org.jgrapht:jgrapht-core:1.5.2")
    implementation("org.jgrapht:jgrapht-io:1.5.2")
}