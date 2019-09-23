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
with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        mapped = [x[0]]
        for i in range(1, len(x)): # x[0] stores the docID
            t = x[i].encode('utf-8')
            try:
                id = tokens[t]
                mapped.append(id)
            except KeyError:
                print("'" + t + "' not found in dictionary")
                print(line)
                exit()

        mapped.append("0") # terminator
        s = [str(i) for i in mapped]
        output_file.write(" ".join(s) + "\n")

        lines += 1
        if lines % 1000000 == 0:
            print("processed " + str(lines) + " lines")
output_file.close()