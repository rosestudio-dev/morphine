plugins {
    kotlin("jvm")
}

group = "ru.unit.morphine.assembly.compiler"

repositories {
    mavenCentral()
}

dependencies {
    implementation(project(":bytecode"))
    implementation(project(":optimizer"))
    implementation(kotlin("stdlib"))
}