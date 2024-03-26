import argparse
from glob import glob
from os.path import isdir
from os.path import isfile

parser = argparse.ArgumentParser(prog='Walker')
parser.add_argument('-p', '--pattern', default='')
parser.add_argument('-d', '--onlydir', default=False)
parser.add_argument('-f', '--onlyfile', default=False)
parser.add_argument('-r', '--recursive', default=True)
args = parser.parse_args()

sources = glob(args.pattern, recursive=args.recursive)
for i in sources:
    if not args.onlydir and not args.onlyfile:
        print(i)
        continue

    if (args.onlyfile and isfile(i)) or (args.onlydir and isdir(i)):
        print(i)
        continue
