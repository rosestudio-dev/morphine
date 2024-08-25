import shutil
import subprocess
import sys

gradle = shutil.which("gradle")
npm = shutil.which("npm")

if gradle is None:
    print("Require gradle")
    exit(1)

if npm is None:
    print("Require npm")
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


step("Install deps", [npm, "i"], "site")
step("Generate docs", [gradle, "run", "--args", "../../components ../site/src/generated kt c"], "gen")
shutil.copy("../changelog.md", "site/src/generated/changelog.md")
step("Deploy", [npm, "run", "docs:dev", "--"] + sys.argv[1:], "site")
