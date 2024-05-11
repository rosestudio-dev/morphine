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
}