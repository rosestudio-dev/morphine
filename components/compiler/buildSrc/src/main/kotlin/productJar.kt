import org.gradle.api.DefaultTask
import org.gradle.api.file.DuplicatesStrategy
import org.gradle.jvm.tasks.Jar
import org.gradle.kotlin.dsl.create
import org.jetbrains.kotlin.gradle.targets.jvm.KotlinJvmTarget

abstract class ProductJarTask : DefaultTask() {

    fun jarTasks() = dependsOn.filterIsInstance<Jar>()

    fun inject(jarTask: Jar) {
        dependsOn(jarTask)

        doFirst {
            with(project) {
                with(jarTask) {
                    manifest.attributes["Implementation-Version"] = version

                    if ("runtimeClasspath" in configurations.names) {
                        from(configurations.getByName("runtimeClasspath").map(::zipTree))
                    }

                    duplicatesStrategy = DuplicatesStrategy.WARN
                }
            }
        }
    }
}

fun KotlinJvmTarget.productJar(): Unit = with(project) {
    val artifactTask = tasks.getByName(artifactsTaskName)

    tasks.create<ProductJarTask>("productJar") {
        inject(artifactTask as Jar)
    }
}
