import sys

input_filename = sys.argv[1] # e.g., "completions.mapped"

nodes_per_level = {} # (level_id, num_nodes)
lines = 0
print("building stats...")

output_file = open(input_filename + ".stats", 'a')
prev = []
universe = 0;
with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        docid = int(x[0])

        if docid > universe:
            universe = docid

        q = x[1:len(x)]

        level_id = 0
        while level_id < len(q) and level_id < len(prev) and q[level_id] == prev[level_id]:
            level_id += 1

        while level_id < len(q):
            if level_id in nodes_per_level:
                nodes_per_level[level_id] += 1
            else:
                nodes_per_level[level_id] = 1
            level_id += 1

        prev = q
        lines += 1
        if lines % 1000000 == 0:
            print("processed " + str(lines) + " lines")

# number of completions
# number of levels in the trie
# number of nodes for each level
print("universe: " + str(universe + 1))
print("completions: " + str(lines))
output_file.write(str(lines) + "\n")
output_file.write(str(universe + 1) + "\n")
output_file.write(str(len(nodes_per_level)) + "\n")
for key, value in sorted(nodes_per_level.iteritems(), key = lambda kv: kv[0]):
    output_file.write(str(value) + "\n")
output_file.close()

