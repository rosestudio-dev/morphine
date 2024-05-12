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
            sharedLib(listOf(RELEASE)) {
                baseName = "sharedcompiler"
            }

            staticLib(listOf(RELEASE)) {
                baseName = "staticcompiler"
            }
        }
    }

    jvm {
        withJava()
        productJar()
    }

    applyDefaultHierarchyTemplate()

    sourceSets {
        commonMain.dependencies {
            implementation(project(":compiler"))
            implementation(kotlin("stdlib"))
        }

        val jvmMain by getting {
            dependencies {
                implementation(project(":compiler"))
                implementation(kotlin("stdlib"))
            }
        }
    }
}
