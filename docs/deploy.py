import os
import shutil
import subprocess
import sys


def error(text):
    print(f"\u001b[31;1m{text}\u001b[0m")
    exit(1)


cwd = os.getcwd()

npm = shutil.which("npm")
meson = shutil.which("meson")
python = shutil.which("python")

if npm is None:
    error("Require npm")

if meson is None:
    error("Require meson")

if python is None:
    error("Require python")


def step(name, args, cwd):
    print(f"\u001b[32;1m-- {name} --\u001b[0m")

    code = subprocess.call(
        args=args,
        cwd=cwd,
        stderr=subprocess.STDOUT
    )

    if code != 0:
        error("-- Failure --")


def insert_gitignore(path):
    print(f"\u001b[32;1m-- Insert gitignore: {path} --\u001b[0m")
    file = open(path + "/.gitignore", "w")
    file.write("*")
    file.close()


def copy(src, dest):
    print(f"\u001b[32;1m-- Copy from '{src}' to '{dest}' --\u001b[0m")
    if os.path.isfile(src):
        if len(os.path.dirname(dest)) > 0:
            os.makedirs(os.path.dirname(dest), exist_ok=True)
        shutil.copy(src, dest)
    else:
        shutil.copytree(src, dest)


def remove(path):
    print(f"\u001b[32;1m-- Remove '{path}' --\u001b[0m")
    if not os.path.exists(path):
        return
    if os.path.isfile(path):
        os.remove(path)
    else:
        shutil.rmtree(path)


def prepare():
    step("Install deps", [npm, "i"], "site")

    remove("site/src/generated")
    remove("site/.vitepress/generated")

    step("Configure morphine",
         [meson, "setup", "docs/wasm/buildmorphine", "-Dprefix=" + cwd + "/wasm/deps",
          "-Dbuild_misc=disabled",
          "-Dbuild_libs=enabled",
          "-Dlibs=math,bigint",
          "-Doptimization=2",
          "-Dbuildtype=release",
          "-Ddefault_library=static",
          "--cross",
          "docs/wasm/cross/wasm.ini"], "..")
    step("Compile morphine", [meson, "install", "--quiet"], "wasm/buildmorphine")
    insert_gitignore("wasm/deps")

    step("Configure wasm",
         [meson, "setup", "buildwasm", "-Doptimization=2", "-Dbuildtype=release", "-Dprefix=" + cwd + "/wasm/out",
          "--cross", "cross/wasm.ini"],
         "wasm")
    step("Compile wasm", [meson, "install", "--quiet"], "wasm/buildwasm")
    insert_gitignore("wasm/out")
    copy("wasm/out/bin/morphine.js", "site/.vitepress/generated/morphine.js")
    copy("wasm/out/bin/morphine.wasm", "site/.vitepress/generated/morphine.wasm")

    step("Generate markdown", [python, "extractor.py", "-p", "../components/**/*.[ch]", "-d", "site/src/generated"],
         ".")
    copy("../changelog.md", "site/src/generated/changelog.md")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        variant = "dev"
    else:
        variant = sys.argv[1]

    if variant == "dev":
        prepare()
        step("Deploy", [npm, "run", "docs:dev", '--'] + sys.argv[2:], "site")
    elif variant == "build":
        prepare()
        step("Build", [npm, "run", "docs:build"], "site")
        remove("dist")
        copy("site/.vitepress/dist", "dist")
        insert_gitignore("dist")
    elif variant == "clean":
        step("Clean", [python, "cleaner.py"], ".")
    else:
        error("Supported variants: dev, build or clean")
