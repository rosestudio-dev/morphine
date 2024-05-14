import com.google.devtools.ksp.gradle.KspTaskMetadata

plugins {
    kotlin("multiplatform")
    id("com.google.devtools.ksp")
}

repositories {
    mavenCentral()
}

kotlin {
    jvm()
    linuxX64()

    applyDefaultHierarchyTemplate()

    sourceSets {
        val commonMain by getting {
            dependencies {
                implementation(kotlin("stdlib"))
                implementation(project(":annotation"))
            }

            tasks.withType<KspTaskMetadata> { kotlin.srcDir(destinationDirectory) }
        }
    }
}

ksp {
    arg("processor.packagePrefix", "morphine.bytecode")
}

dependencies {
    add("kspCommonMainMetadata", project(":processor"))
}
