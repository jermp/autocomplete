import sys, os

type = sys.argv[1] # 'trie' or 'fc'
collection_basename = sys.argv[2]
num_queries = sys.argv[3]

output_filename = collection_basename + "." + type + ".locate_prefix.json"
query_filename_prefix = collection_basename + ".queries/queries."

percentages = ["0.0", "0.25", "0.50", "0.75"]
for perc in percentages:
    for terms in range(1,8):
        os.system("../build/benchmark_locate_prefix " + type + " " + collection_basename + " " + str(terms) + " " + str(num_queries) + " " + perc + " < " + query_filename_prefix + "length=" + str(terms) + " 2>> " + output_filename)
    os.system("../build/benchmark_locate_prefix " + type + " " + collection_basename + " 8+ " + str(num_queries) + " " + perc + " < " + query_filename_prefix + "length=8+ 2>> " + output_filename)
