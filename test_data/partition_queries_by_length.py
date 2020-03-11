import sys, os, random

input_filename = sys.argv[1]
output_directory = sys.argv[2]
n = int(sys.argv[3])

if not os.path.exists(output_directory):
    os.makedirs(output_directory)

num_shards = 6
files = [open(output_directory + "/queries.length=" + str(i), "w") for i in range(1,num_shards + 1)]
all_others = open(output_directory + "/queries.length=" + str(num_shards + 1) + "+", "w")

strings = [[] for i in range(num_shards)]
all_others_strings = []

lines = 0
with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        l = len(x) - 1
        string = ' '.join(x[1:l+1]) + '\n'
        if l > num_shards:
            all_others_strings.append(string)
        else:
            strings[l - 1].append(string)
        lines += 1
        if lines % 1000000 == 0:
            print("processed " + str(lines) + " lines")

for i in range(num_shards):
    random.shuffle(strings[i])
    for k in range(min(n, len(strings[i]))):
        files[i].write(strings[i][k])
    files[i].close()

random.shuffle(all_others_strings)
for k in range(min(n, len(all_others_strings))):
    all_others.write(all_others_strings[k])
all_others.close()
