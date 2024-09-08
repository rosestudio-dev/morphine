import glob
import os
import pathlib
import shutil


def remove(base, path):
    if path is None:
        fullpath = base
    else:
        fullpath = os.path.join(base, path)

    files = glob.glob(str(fullpath))
    for file in files:
        if not os.path.exists(file):
            print(f"\u001b[33mNot found: {file}\u001b[0m")

        if os.path.isdir(file):
            shutil.rmtree(file)
        else:
            os.remove(file)


def clean(gitignore):
    base = pathlib.Path(gitignore).parent
    with open(gitignore, "r") as file:
        content = file.read()
    for line in content.splitlines():
        line = line.strip()
        if line.startswith("#"):
            continue
        elif line == "*":
            remove(base, None)
            break
        else:
            remove(base, line)


if __name__ == "__main__":
    sources = glob.glob("**/.gitignore", recursive=True)
    for path in sources:
        clean(path)
