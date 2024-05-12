import org.gradle.api.Project

fun Project.projectFullName(): String {
    val projects = mutableListOf<Project>()

    var current: Project? = this
    while (current != null) {
        projects.add(current)
        current = current.parent
    }

    return projects.reversed().joinToString(separator = "_", transform = Project::getName)
}