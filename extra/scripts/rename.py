import argparse
import shutil

parser = argparse.ArgumentParser(prog='Renamer')
parser.add_argument('-i', '--inputs', default='')
parser.add_argument('-o', '--outputs', default='')
args = parser.parse_args()

inputs = args.inputs.split(":")
outputs = args.outputs.split(":")

for i in range(len(inputs)):
    if inputs[i] == outputs[i]:
        continue

    shutil.copy(inputs[i], outputs[i])
