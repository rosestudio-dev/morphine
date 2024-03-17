plugins {
    kotlin("jvm")
    id("com.google.devtools.ksp")
}

group = "ru.unit.morphine.assembly.bytecode"

repositories {
    mavenCentral()
}

kotlin.sourceSets.main {
    kotlin.srcDirs(file("${getBuildFile().path}/generated/ksp/main/kotlin"))
}

ksp {
    arg("bytecode-processor.packagePrefix", group.toString())
}

dependencies {
    implementation(kotlin("stdlib"))
    implementation(project(":bytecode-annotation"))
    ksp(project(":bytecode-processor"))
}