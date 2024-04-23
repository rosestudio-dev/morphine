plugins {
    application
    kotlin("jvm") version "1.9.23"
}

group = "ru.why.gen"
version = "1.0"

repositories {
    mavenCentral()
}

dependencies {
}

application {
    mainClass.set("ru.why.gen.MainKt")
}
