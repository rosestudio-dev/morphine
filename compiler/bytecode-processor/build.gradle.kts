plugins {
    kotlin("jvm")
}

group = "ru.unit.morphine.assembly.bytecode.processor"

repositories {
    mavenCentral()
}

dependencies {
    implementation(project(":bytecode-annotation"))
    implementation(kotlin("stdlib"))
    implementation("com.google.devtools.ksp:symbol-processing-api:1.8.22-1.0.11")
    implementation("com.squareup:kotlinpoet:1.14.2")
    implementation("com.squareup:kotlinpoet-ksp:1.14.2")
}