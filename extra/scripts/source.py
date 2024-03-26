from glob import glob
from sys import argv

if len(argv) > 1:
    prefix = argv[1] + "/"
else:
    prefix = ""

sources = glob(prefix + '**/*.c', recursive=True)
for i in sources:
    print(i)

