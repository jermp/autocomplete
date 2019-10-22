import sys
from sets import Set

input_filename = sys.argv[1]

tokens = Set({})
lines = 0

print("parsing input file...")
with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        for i in range(1, len(x)): # x[0] stores the docID
            tokens.add(x[i])
        lines += 1
        if lines % 1000000 == 0:
            print "processed " + str(lines) + " lines"

print("processed " + str(lines) + " lines")
print("dictionary has " + str(len(tokens)) + " keys")

dict_file = open(input_filename + ".dict", 'w')
for key in sorted(tokens):
    dict_file.write(key + "\n")
dict_file.close()