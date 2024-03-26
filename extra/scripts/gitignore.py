from sys import argv

file = open(argv[1] + '/.gitignore', 'w')
file.write('*')
file.close()

