import sys, os

index_type = sys.argv[1]
query_mode = sys.argv[2] # topk, prefix_topk, conjunctive_topk
index_filename = sys.argv[3]
dataset_basename = sys.argv[4] # e.g., aol/aol.completions or aol/aol.completions.filtered
k = sys.argv[5]
num_queries = sys.argv[6]

output_filename = dataset_basename + "." + index_type

breakdown = ""
if len(sys.argv) > 7 and sys.argv[7] == "--breakdown":
    breakdown = "--breakdown"
    output_filename += ".breakdown"

output_filename += "." + query_mode + ".json"
query_filename_prefix = dataset_basename + ".queries/queries."

percentages = ["0.0", "0.25", "0.50", "0.75"]
for perc in percentages:
    for terms in range(2,8): # (1,8)
        os.system("../build/benchmark_" + query_mode + " " + index_type + " " + k + " ../build/" + index_filename + " " + str(terms) + " " + str(num_queries) + " " + perc + " " + breakdown + " < " + query_filename_prefix + "length=" + str(terms) + " 2>> " + output_filename)
    os.system("../build/benchmark_" + query_mode + " " + index_type + " " + k + " ../build/" + index_filename + " 8+ " + str(num_queries) + " " + perc + " " + breakdown + " < " + query_filename_prefix + "length=8+ 2>> " + output_filename)
