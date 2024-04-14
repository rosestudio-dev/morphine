plugins {
    application
    kotlin("jvm")
    id("com.github.gmazzo.buildconfig")
}

group = "ru.unit.morphine.assembly.cli.compiler"

repositories {
    mavenCentral()
}

application {
    mainClass.set("ru.unit.morphine.assembly.cli.compiler.MainKt")
}

buildConfig {
    buildConfigField("version", rootProject.ext.get("version").toString())
    buildConfigField("versionCode", rootProject.ext.get("versionCode").toString().toInt())
}

tasks.jar {
    manifest.attributes["Main-Class"] = application.mainClass
    manifest.attributes["Implementation-Version"] = version

    val dependencies = configurations
        .runtimeClasspath
        .get()
        .map(::zipTree)

    from(dependencies)
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE

    archiveBaseName.set("cli")

    System.getProperty("jarOutputDir")?.let { jarOutputDir ->
        val dir = File(jarOutputDir).apply { mkdir() }

        destinationDirectory.set(dir)
    }
}

dependencies {
    implementation(project(":bytecode"))
    implementation(project(":compiler"))
    implementation(kotlin("stdlib"))
    implementation("org.jetbrains.kotlinx:kotlinx-cli:0.3.6")
}
