package ru.why.gen

import java.io.File
import java.nio.file.Files

fun main(args: Array<String>) {
    val searchBase = File(args[0])
    val outputBase = File(args[1])

    outputBase.deleteRecursively()
    walking(
        searchBase = searchBase,
        outputBase = outputBase,
        extensions = args.drop(2),
        base = searchBase
    )
}

private fun walking(searchBase: File, outputBase: File, extensions: List<String>, base: File) {
    searchBase.listFiles()?.forEach { file ->
        if (file.isDirectory) {
            walking(
                searchBase = file,
                outputBase = outputBase,
                extensions = extensions,
                base = base
            )
        } else if (extensions.any { extension -> file.name.endsWith(".$extension") }) {
            watch(
                file = file,
                outputBase = outputBase,
                base = base
            )
        }
    }
}

private const val OPEN_TAG_PATTERN = "\\{\\{docs (header|append)}}"
private const val CLOSE_TAG_PATTERN = "\\{\\{end}}"
private const val PATH_PATTERN = "path:\\S+"

private fun watch(file: File, outputBase: File, base: File) {
    val text = Files.readString(file.toPath())
    val regex = "$OPEN_TAG_PATTERN(.|\\s)*?$PATH_PATTERN?(.|\\s)*?$CLOSE_TAG_PATTERN".toRegex(RegexOption.MULTILINE)

    val regions = mutableMapOf<String, List<Region>>()
    regex.findAll(text).forEach { matchResult ->
        val prefix = prefix(
            text = text,
            first = matchResult.range.first
        )

        val (path, region) = region(
            found = matchResult.value,
            prefix = prefix
        )

        regions[path] = (regions[path] ?: listOf()) + region
    }

    if (regions.isNotEmpty()) {
        val count = regions.values.flatten().count()
        val relativePath = file.canonicalPath.removePrefix(base.canonicalPath + File.separator)
        println("${relativePath}: Found $count")

        regions.forEach { (path, regions) ->
            val output = File(outputBase, path)
            val fileText = regions.sortedByDescending(Region::isHeader)
                .joinToString(
                    separator = "\n\n",
                    transform = Region::text
                )

            output.parentFile.mkdirs()
            Files.writeString(
                output.toPath(),
                fileText
            )
        }
    }
}

private fun prefix(text: String, first: Int): String {
    val newLineIndex = (first downTo 0).find { index ->
        text[index] in "\n\r"
    } ?: -1

    return text.substring(newLineIndex + 1, first)
}

private fun region(found: String, prefix: String): Pair<String, Region> {
    val lines = found.lines().map { line -> line.removePrefix(prefix) }

    val tagOpen = lines.first()
    val tagClose = lines.last()
    val path = lines[1]

    // checks
    val tagOpenRegex = OPEN_TAG_PATTERN.toRegex()
    val pathRegex = PATH_PATTERN.toRegex()
    if (!tagOpenRegex.matches(tagOpen)) {
        throw RuntimeException("Open tag is corrupted")
    }

    if (tagClose != "{{end}}") {
        throw RuntimeException("Close tag is corrupted")
    }

    if (!pathRegex.matches(path)) {
        throw RuntimeException("Path is corrupted")
    }

    val extractedPath = path.removePrefix("path:") + ".md"

    val text = lines.drop(2).dropLast(1).joinToString(separator = "\n")
    val isHeader = when (tagOpenRegex.find(tagOpen)?.groups?.get(1)?.value) {
        "header" -> true
        "append" -> false
        else -> throw RuntimeException("Unknown mode")
    }

    return extractedPath to Region(
        isHeader = isHeader,
        text = text
    )
}

private data class Region(
    val isHeader: Boolean,
    val text: String,
)
