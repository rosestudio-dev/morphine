package ru.why.gen

import java.io.File
import java.nio.file.Files

fun main(args: Array<String>) {
    val searchBase = File(args[0])
    val outputBase = File(args[1])

    val regions = mutableMapOf<String, List<Region>>()

    outputBase.deleteRecursively()

    walking(
        searchBase = searchBase,
        extensions = args.drop(2),
        base = searchBase,
        regions = regions,
    )

    generate(
        regions = regions,
        outputBase = outputBase
    )
}

private fun walking(
    searchBase: File,
    extensions: List<String>,
    base: File,
    regions: MutableMap<String, List<Region>>
) {
    searchBase.listFiles()?.forEach { file ->
        if (file.isDirectory) {
            walking(
                searchBase = file,
                extensions = extensions,
                base = base,
                regions = regions,
            )
        } else if (extensions.any { extension -> file.name.endsWith(".$extension") }) {
            watch(
                file = file,
                base = base,
                regions = regions,
            )
        }
    }
}

private const val OPEN_TAG_PATTERN = "(.*)\\{\\{docs (.*?)}}"
private const val CLOSE_TAG_PATTERN = "(.*)\\{\\{end}}"
private const val PATH_PATTERN = "(.*)path:(.+)"

private fun watch(
    file: File,
    base: File,
    regions: MutableMap<String, List<Region>>
) {
    val relativePath = file.canonicalPath.removePrefix(base.canonicalPath + File.separator)
    val text = Files.readString(file.toPath())
    val lines = text.lines()

    val openTagRegex = OPEN_TAG_PATTERN.toRegex()
    val count = lines.withIndex().count { (index, line) ->
        val match = openTagRegex.matchEntire(line)
        if (match != null) {
            val (path, region) = region(
                match = match,
                indexLine = index,
                lines = lines,
                relativePath = relativePath
            )

            regions[path] = (regions[path] ?: listOf()) + region
        }

        match != null
    }

    if (count > 0) {
        println("${relativePath}: Found $count")
    }
}

private fun region(
    match: MatchResult,
    indexLine: Int,
    lines: List<String>,
    relativePath: String
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
        text = text,
        relativePath = relativePath
    )
}

private const val HEADER = "---\nlayout: doc\n---\n\n"

private fun generate(
    regions: MutableMap<String, List<Region>>,
    outputBase: File
) {
    regions.forEach { (path, regions) ->
        val output = File(outputBase, path)
        val fileText = regions.sortedBy { region ->
            region.place.ordinal
        }.joinToString(
            separator = "\n\n",
            transform = Region::text
        )

        val sourceFiles = regions.map(Region::relativePath)
            .distinct().joinToString(separator = "\n")

        val genFrom = "<!--\nGenerated from:\n$sourceFiles\n-->\n\n"

        output.parentFile.mkdirs()
        Files.writeString(
            output.toPath(),
            "$HEADER$genFrom$fileText"
        )
    }
}

private data class Region(
    val place: Place,
    val text: String,
    val relativePath: String
) {
    enum class Place {
        HEADER,
        BODY,
        FOOTER
    }
}
