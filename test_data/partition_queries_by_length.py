import sys
import numpy as np
import random

input_filename = sys.argv[1]

num_shards = 7
files = [open(input_filename + ".length=" + str(i) + ".shuffled", "w") for i in range(1,num_shards + 1)]
all_others = open(input_filename + ".length=" + str(num_shards + 1) + "+.shuffled", "w")

strings = [[] for i in range(0, num_shards)]
all_others_strings = []

lines = 0
with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        l = len(x) - 1

        if l > num_shards:
            all_others_strings.append(line)
        else:
            strings[l - 1].append(line)

        lines += 1
        if lines % 1000000 == 0:
            print("processed " + str(lines) + " lines")


for i in range(0, num_shards):
    random.shuffle(strings[i])
    for s in strings[i]:
        files[i].write(s)
    files[i].close()

random.shuffle(all_others_strings)
for s in all_others_strings:
    all_others.write(s)
all_others.close()

