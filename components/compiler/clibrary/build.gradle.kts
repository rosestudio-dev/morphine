plugins {
    kotlin("jvm")
    id("com.github.gmazzo.buildconfig")
}

group = "ru.unit.morphine.assembly.clibrary"

repositories {
    mavenCentral()
}

buildConfig {
    buildConfigField("version", rootProject.ext.get("version").toString())
    buildConfigField("versionCode", rootProject.ext.get("versionCode").toString().toInt())
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