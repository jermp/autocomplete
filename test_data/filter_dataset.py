import sys
from sets import Set

input_filename = sys.argv[1]
queries_directory = sys.argv[2]

to_filter = Set({})
print("loading strings to filter...")
for i in range(1,7):
    with open(queries_directory + "/queries.length=" + str(i)) as f:
        for line in f:
            s = line.rstrip('\n')
            to_filter.add(s)
with open(queries_directory + "/queries.length=7+") as f:
    for line in f:
        s = line.rstrip('\n')
        to_filter.add(s)

lines = 0
print("filtering dataset...")

output_file = open(input_filename + ".filtered", 'w')
with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        string = ' '.join(x[1:len(x)])
        if string not in to_filter:
            output_file.write(line)
        lines += 1
        if lines % 1000000 == 0:
            print("processed " + str(lines) + " lines")
output_file.close()