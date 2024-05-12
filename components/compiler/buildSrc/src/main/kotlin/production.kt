import java.io.File
import org.gradle.api.Project
import org.gradle.api.file.DuplicatesStrategy
import org.gradle.api.tasks.Copy
import org.gradle.jvm.tasks.Jar
import org.gradle.kotlin.dsl.create
import org.jetbrains.kotlin.gradle.plugin.mpp.NativeBuildType
import org.jetbrains.kotlin.gradle.plugin.mpp.NativeOutputKind
import org.jetbrains.kotlin.gradle.tasks.KotlinNativeLink

fun Project.production() {
    afterEvaluate {
        val output = if (Properties.outputDir.isNotBlank()) {
            File(Properties.outputDir)
        } else {
            File(rootDir, "production")
        }

        tasks.create<Copy>("production") {
            val linkTasks = tasks.filterIsInstance<KotlinNativeLink>().filter { task ->
                task.binary.outputKind != NativeOutputKind.TEST && task.binary.buildType == NativeBuildType.RELEASE
            }

            val productJarTasks = tasks.filterIsInstance<ProductJarTask>()

            val linkInputs = linkTasks.map(KotlinNativeLink::destinationDirectory)
            val jarInputs = productJarTasks.flatMap(ProductJarTask::jarTasks).map(Jar::getDestinationDirectory)

            dependsOn(linkTasks + productJarTasks)

            duplicatesStrategy = DuplicatesStrategy.WARN
            from(linkInputs + jarInputs)
            into(output)
        }
    }
}