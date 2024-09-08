import argparse
import enum
import glob
import pathlib
import re

OPEN_TAG_PATTERN = r"(.*)\{\{docs (.*?)}}"
CLOSE_TAG_PATTERN = r"(.*)\{\{end}}"
PATH_PATTERN = r"(.*)path:(.+)"
FILE_HEADER = "---\nlayout: doc\n---\n\n"


class Place(enum.Enum):
    HEADER = 0
    BODY = 1
    FOOTER = 2


def region(file_path, prefix, line_index, content):
    place = None
    path = None
    close = None

    index = 0
    for line in content.splitlines()[line_index:]:
        if index == 0:
            matched = re.match(OPEN_TAG_PATTERN, line)
            if matched is None:
                raise Exception("Failed to parse open tag")
            place = matched.group(2)
        else:
            if re.match(OPEN_TAG_PATTERN, line) is not None:
                raise Exception("Failed to parse pattern")

            if index == 1:
                matched = re.match(PATH_PATTERN, line)
                if matched is None:
                    raise Exception("Failed to parse path")
                path = matched.group(2)
            elif re.match(CLOSE_TAG_PATTERN, line) is not None:
                close = index + line_index
                break
        index += 1

    if (place is None) or (path is None) or (close is None):
        raise Exception("Corrupted pattern")

    def prefix_remover(line):
        if line.startswith(prefix):
            return line.removeprefix(prefix)
        elif line.rstrip() == prefix.rstrip():
            return ""
        else:
            return line.rstrip()

    text = "\n".join(map(prefix_remover, content.splitlines()[line_index + 2:close]))

    match place:
        case "header":
            place_type = Place.HEADER
        case "body":
            place_type = Place.BODY
        case "footer":
            place_type = Place.FOOTER
        case _:
            raise Exception("Unknown place")

    return path.strip(), {
        "place": place_type,
        "text": text,
        "path": file_path
    }


def watch(regions, path, content):
    count = 0
    index = 0
    for line in content.splitlines():
        matched = re.match(OPEN_TAG_PATTERN, line)
        if matched is not None:
            prefix = matched.group(1)
            gen_path, reg = region(path, prefix, index, content)
            if regions.get(gen_path) is None:
                regions[gen_path] = []
            regions[gen_path].append(reg)
            count += 1

        index += 1
    if count > 0:
        print(f"{path}: Found {count}")


def scanfile(regions, path):
    with open(path, "r") as file:
        content = file.read()
    watch(regions, path, content)


def generate(regions, dest_path):
    def sortPlace(value):
        return value["place"].value

    def mapText(value):
        return value["text"]

    def mapPath(value):
        return value["path"]

    for path, regions in regions.items():
        file_text = "\n\n".join(map(mapText, sorted(regions, key=sortPlace)))
        sources = "\n".join(set(map(mapPath, regions)))
        gen_from = f"<!--\nGenerated from:\n{sources}\n-->\n\n"

        fullpath = pathlib.Path(dest_path).joinpath(f"{path}.md")
        fullpath.parent.mkdir(parents=True, exist_ok=True)
        with open(fullpath, "w") as file:
            file.write(f"{FILE_HEADER}{gen_from}{file_text}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='Extractor')
    parser.add_argument('-p', '--pattern', default='')
    parser.add_argument('-d', '--dest', default='')
    args = parser.parse_args()

    if len(args.pattern.strip()) == 0:
        raise Exception("Empty pattern")

    if len(args.dest.strip()) == 0:
        raise Exception("Empty dest")

    regions = {}
    sources = glob.glob(args.pattern, recursive=True)
    for path in sources:
        scanfile(regions, path)

    generate(regions, args.dest)
