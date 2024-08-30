import os
import shutil
import subprocess
import sys

cwd = os.getcwd()

npm = shutil.which("npm")
meson = shutil.which("meson")

if npm is None:
    print("Require npm")
    exit(1)

if npm is None:
    print("Require meson")
    exit(1)


def step(name, args, cwd):
    print("\u001b[32;1m-- " + name + " --\u001b[0m")

    code = subprocess.call(
        args=args,
        cwd=cwd,
        stderr=subprocess.STDOUT
    )

    if code != 0:
        print("\u001b[31;1m-- Failure --\u001b[0m")
        exit(1)


def insert_gitignore(path):
    file = open(path + "/.gitignore", "w")
    file.write("*")
    file.close()


def copy(src, dest):
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    shutil.copy(src, dest)


def prepare():
    step("Install deps", [npm, "i"], "site")

    step("Configure morphine",
         [meson, "setup", "docs/wasm/buildmorphine", "-Dprefix=" + cwd + "/wasm/deps", "-Ddefault_library=static",
          "-Dbuildapp=disabled", "-Dbuildlibs=disabled", "-Doptimization=2", "-Dbuildtype=release", "--cross",
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

    # step("Generate docs", [gradle, "run", "--args", "../../components ../site/src/generated kt c"], "gen")
    copy("../changelog.md", "site/src/generated/changelog.md")


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
else:
    print("\u001b[31;1mSupported variants: dev, build\u001b[0m")
    exit(1)
