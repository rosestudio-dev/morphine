import argparse
import sys

parser = argparse.ArgumentParser(prog='Cmake generator')
parser.add_argument('-s', '--sources', default='')
parser.add_argument('-i', '--publicincludes', default='')
parser.add_argument('-p', '--privateincludes', default='')
parser.add_argument('-o', '--output', default='')
parser.add_argument('-v', '--version', default='')
parser.add_argument('-c', '--versioncode', default='')
args = parser.parse_args()

sources = args.sources.split(":")
private_includes = args.privateincludes.split(":")
public_includes = args.publicincludes.split(":")

if args.output == "":
    print("Empty output", file=sys.stderr)
    exit(1)

f = open(args.output, "w")

f.write(
    """cmake_minimum_required(VERSION 3.16)
project(morphine)

include_directories(
"""
)

for i in (private_includes + public_includes):
    f.write("    " + i + "\n")

f.write(
    """)

add_library(
    morphine STATIC
"""
)

for i in sources:
    f.write("    " + i + "\n")

f.write(
    """)

target_include_directories(
    morphine INTERFACE
"""
)

for i in public_includes:
    f.write("    " + i + "\n")

f.write(
    """)

target_compile_definitions(
    morphine PUBLIC
    MORPHINE_VERSION=\""""
)
f.write(args.version)
f.write(
    """\"
    MORPHINE_VERSION_CODE="""
)
f.write(args.versioncode)
f.write(
    """
)
"""
)

f.close()
