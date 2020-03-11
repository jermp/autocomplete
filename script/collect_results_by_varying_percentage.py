import sys, os

index_type = sys.argv[1]
query_mode = sys.argv[2] # topk, prefix_topk, conjunctive_topk
index_filename = sys.argv[3]
collection_basename = sys.argv[4] # e.g., aol/aol.completions or aol/aol.completions.filtered
k = sys.argv[5]
num_queries = sys.argv[6]

output_filename = collection_basename + "." + index_type
output_filename += "." + query_mode + ".json"
query_filename_prefix = collection_basename + ".queries/queries."

percentages = ["0.0", "0.25", "0.50", "0.75"]
for perc in percentages:
    for terms in range(1,7):
        os.system("../build/benchmark_" + query_mode + " " + index_type + " " + k + " ../build/" + index_filename + " " + str(terms) + " " + str(num_queries) + " " + perc + " < " + query_filename_prefix + "length=" + str(terms) + " 2>> " + output_filename)
    os.system("../build/benchmark_" + query_mode + " " + index_type + " " + k + " ../build/" + index_filename + " 7+ " + str(num_queries) + " " + perc + " < " + query_filename_prefix + "length=7+ 2>> " + output_filename)
