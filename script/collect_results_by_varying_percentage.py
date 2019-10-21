import sys, os

type = sys.argv[1]
index_filename = sys.argv[2]
dataset_name = sys.argv[3]
k = sys.argv[4]
num_queries = sys.argv[5]

output_filename = dataset_name + "." + type

breakdown = ""
if len(sys.argv) > 6 and sys.argv[6] == "--breakdown":
    breakdown = "--breakdown"
    output_filename += ".breakdown"

output_filename += ".topk.timings.json"

percentages = ["0.0", "0.25", "0.50", "0.75"]

for perc in percentages:
    for terms in range(2,8): # (1,8)
        os.system("../build/benchmark_topk " + type + " " + k + " ../build/" + index_filename + " " + str(terms) + " " + str(num_queries) + " " + perc + " " + breakdown + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=" + str(terms) + ".shuffled 2>> " + output_filename)
    os.system("../build/benchmark_topk " + type + " " + k + " ../build/" + index_filename + " 8+ " + str(num_queries) + " " + perc + " " + breakdown + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=8+.shuffled 2>> " + output_filename)
