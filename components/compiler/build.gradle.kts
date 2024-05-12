group = "morphine"

plugins {
    kotlin("multiplatform").apply(false)
    id("com.google.devtools.ksp").apply(false)
    id("com.github.gmazzo.buildconfig").apply(false)
}

subprojects {
    if (Properties.monoBuildDir.isNotBlank()) {
        val monoBuildDir = File(
            File(Properties.monoBuildDir).apply { mkdir() },
            project.projectFullName()
        )

        layout.buildDirectory.set(monoBuildDir)
    }

    version = Properties.projectVersion

    production()
}
