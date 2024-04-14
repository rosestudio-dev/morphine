plugins {
    kotlin("jvm")
    id("com.google.devtools.ksp")
    id("com.github.gmazzo.buildconfig")
}

group = "ru.unit.morphine.assembly.bytecode"

repositories {
    mavenCentral()
}

buildConfig {
    buildConfigField("version", rootProject.ext.get("version").toString())
    buildConfigField("versionCode", rootProject.ext.get("versionCode").toString().toInt())
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