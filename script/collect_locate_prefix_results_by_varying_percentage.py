import sys, os

type = sys.argv[1] # 'trie' or 'fc'
collection_basename = sys.argv[2]
dataset_name = sys.argv[3]
num_queries = sys.argv[4]

output_filename = dataset_name + "." + type + ".locate_prefix.timings.json"

percentages = ["0.0", "0.25", "0.50", "0.75"]

for perc in percentages:
    for terms in range(2,8): # (1,8)
        os.system("../build/benchmark_locate_prefix " + type + " " + collection_basename + " " + str(terms) + " " + str(num_queries) + " " + perc + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=" + str(terms) + ".shuffled 2>> " + output_filename)
    os.system("../build/benchmark_locate_prefix " + type + " " + collection_basename + " 8+ " + str(num_queries) + " " + perc + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=8+.shuffled 2>> " + output_filename)
