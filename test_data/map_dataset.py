import sys

input_filename = sys.argv[1]

tokens = {}
print("building dictionary...")
id = 1 # reserve id 0 to mark the end of a string
with open(input_filename + ".dict") as f:
    for line in f:
        t = line.rstrip('\n')
        tokens[t] = id
        id += 1

lines = 0
print("mapping dataset...")

output_file = open(input_filename + ".mapped", 'w')
stats_file = open(input_filename + ".mapped.stats", 'w')
max_string_len = 0;

with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        string_len = 0;
        mapped = [x[0]]
        for i in range(1, len(x)): # x[0] stores the docID
            t = x[i]
            try:
                id = tokens[t]
                mapped.append(id)
                string_len += len(t)
            except KeyError:
                print("'" + t + "' not found in dictionary")
                print(line)
                exit()

        if string_len > max_string_len:
            max_string_len = string_len

        mapped.append("0") # terminator
        s = [str(i) for i in mapped]
        output_file.write(" ".join(s) + "\n")

        lines += 1
        if lines % 1000000 == 0:
            print("processed " + str(lines) + " lines")
output_file.close()

stats_file.write(str(len(tokens)) + "\n")
stats_file.write(str(max_string_len) + "\n")
stats_file.close()
