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
inverted = open(input_filename + ".inverted", 'w')
forward = open(input_filename + ".forward", 'w')

num_terms = 0
num_docs = 0
with open(input_filename + ".mapped.stats") as f:
    num_terms = int(f.readline())
    print("terms: " + str(num_terms))
    f.readline() # skip line: max num. of query terms
    f.readline() # skip line: num. of completions
    num_docs = int(f.readline())
    print("universe: " + str(num_docs))

inverted_index = [[] for i in range(num_terms + 1)] # id 0 is not assigned
forward_index = [[] for i in range(num_docs)]

with open(input_filename, 'r') as f:
    for line in f:
        x = line.rstrip('\n').split()
        mapped = []
        doc_id = int(x[0])
        discard = False
        for i in range(1, len(x)):
            try:
                term = x[i]
                try:
                    term_id = tokens[term]
                    if term_id not in mapped:
                        inverted_index[term_id].append(doc_id)
                    mapped.append(term_id)
                except KeyError:
                    print("'" + term + "' not found in dictionary")
                    print(line)
                    exit()
            except UnicodeDecodeError:
                discard = True

        if not discard:
            # NOTE: not sorted!
            if doc_id >= num_docs:
                print doc_id,num_docs
            forward_index[doc_id] = mapped;

            lines += 1
            if lines % 1000000 == 0:
                print("processed " + str(lines) + " lines")

for i in range(0, num_docs):
    s = [str(k) for k in forward_index[i]]
    forward.write(str(len(forward_index[i])) + " ")
    forward.write(" ".join(s) + "\n")
forward.close()

for i in range(1, num_terms + 1):
    posting_list = inverted_index[i]
    unique = sorted(set(posting_list));
    s = [str(i) for i in unique] # remove any possible duplicate
    inverted.write(str(len(unique)) + " ")
    inverted.write(" ".join(s) + "\n")
inverted.close()
