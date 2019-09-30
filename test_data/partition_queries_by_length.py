import sys
import numpy as np

input_filename = sys.argv[1]

num_shards = 7
files = [open(input_filename + ".length=" + str(i), "w") for i in range(1,num_shards + 1)]
all_others = open(input_filename + ".length=" + str(num_shards + 1) + "+", "w")

lines = 0
with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        l = len(x) - 1

        if l > num_shards:
            all_others.write(line)
        else:
            files[l - 1].write(line)

        lines += 1
        if lines % 1000000 == 0:
            print("processed " + str(lines) + " lines")

for f in files:
    f.close()
all_others.close()
