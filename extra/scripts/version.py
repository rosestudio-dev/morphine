import argparse
import sys

parser = argparse.ArgumentParser(prog='Cmake generator')
parser.add_argument('-n', '--name', default='')
parser.add_argument('-c', '--code', default='')
parser.add_argument('-b', '--bytecode', default='')
parser.add_argument('-o', '--output', default='')
args = parser.parse_args()

if args.output == "":
    print("Empty output", file=sys.stderr)
    exit(1)

f = open(args.output, "w")
f.write("""
//
// Generated by version.py
//

#pragma once

#define MORPHINE_VERSION_NAME     \"""" + args.name + """\"
#define MORPHINE_VERSION_CODE     """ + args.code + """
#define MORPHINE_BYTECODE_VERSION """ + args.bytecode + """
""")

f.close()
