plugins {
    kotlin("multiplatform")
    id("com.github.gmazzo.buildconfig")
}

repositories {
    mavenCentral()
}

buildConfig {
    buildConfigField("version", Properties.projectVersion)
    buildConfigField("versionCode", Properties.projectVersionCode)
}

kotlin {
    linuxX64 {
        binaries {
            executable(listOf(RELEASE)) {
                entryPoint = "main"
            }
        }
    }

    applyDefaultHierarchyTemplate()

    sourceSets {
        commonMain.dependencies {
            implementation(project(":compiler"))
            implementation(kotlin("stdlib"))
            implementation("org.jetbrains.kotlinx:kotlinx-cli:0.3.6")
            implementation("org.jetbrains.kotlinx:kotlinx-io-core:0.3.4")
        }
    }
}


