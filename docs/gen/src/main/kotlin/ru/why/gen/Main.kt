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

private const val OPEN_TAG_PATTERN = "(.*)\\{\\{docs (header|body|footer)}}"
private const val CLOSE_TAG_PATTERN = "(.*)\\{\\{end}}"
private const val PATH_PATTERN = "(.*)path:(.+)"

private fun watch(file: File, outputBase: File, base: File) {
    val text = Files.readString(file.toPath())
    val lines = text.lines()

    val regions = mutableMapOf<String, List<Region>>()

    val openTagRegex = OPEN_TAG_PATTERN.toRegex()
    lines.forEachIndexed { index, line ->
        val match = openTagRegex.matchEntire(line)
        if (match != null) {
            val (path, region) = region(
                match = match,
                indexLine = index,
                lines = lines
            )

            regions[path] = (regions[path] ?: listOf()) + region
        }
    }

    if (regions.isNotEmpty()) {
        val count = regions.values.flatten().count()
        val relativePath = file.canonicalPath.removePrefix(base.canonicalPath + File.separator)
        println("${relativePath}: Found $count")

        regions.forEach { (path, regions) ->
            val output = File(outputBase, path)
            val fileText = regions.sortedBy { region ->
                region.place.ordinal
            }.joinToString(
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

private fun region(
    match: MatchResult,
    indexLine: Int,
    lines: List<String>
): Pair<String, Region> {
    val prefix = match.groups[1]?.value ?: throw RuntimeException("Failed to parse prefix")
    val openTagRegex = OPEN_TAG_PATTERN.toRegex()
    val closeTagRegex = CLOSE_TAG_PATTERN.toRegex()
    val pathTagRegex = PATH_PATTERN.toRegex()

    var place: String? = null
    var path: String? = null
    var close: Int? = null
    for ((index, line) in lines.drop(indexLine).withIndex()) {
        if (index == 0) {
            val matched = openTagRegex.matchEntire(line)
            if (matched == null) {
                throw RuntimeException("Failed to parse open tag")
            }

            place = matched.groups[2]?.value ?: throw RuntimeException("Failed to parse place")
        } else {
            if (openTagRegex.matches(line)) {
                throw RuntimeException("Failed to parse pattern")
            }

            if (index == 1) {
                val matched = pathTagRegex.matchEntire(line)
                if (matched == null) {
                    throw RuntimeException("Failed to parse path")
                }

                path = matched.groups[2]?.value ?: throw RuntimeException("Failed to parse path string")
            } else if (closeTagRegex.matches(line)) {
                close = index + indexLine
                break
            }
        }
    }

    if (place == null || path == null || close == null) {
        throw RuntimeException("Corrupted pattern")
    }

    val text = lines.subList(indexLine + 2, close).joinToString(separator = "\n") { line ->
        if (line.startsWith(prefix)) {
            line.removePrefix(prefix)
        } else if (line == prefix.trimEnd()) {
            ""
        } else {
            line
        }
    }

    val parsedPlace = when (place) {
        "header" -> Region.Place.HEADER
        "body" -> Region.Place.BODY
        "footer" -> Region.Place.FOOTER
        else -> throw RuntimeException("Unknown place")
    }

    return "${path.trim()}.md" to Region(
        place = parsedPlace,
        text = text
    )
}

private data class Region(
    val place: Place,
    val text: String,
) {
    enum class Place {
        HEADER,
        BODY,
        FOOTER
    }
}
