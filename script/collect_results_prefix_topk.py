import sys, os

type = sys.argv[1]
dataset_name = sys.argv[2]
k = sys.argv[3]
num_queries = sys.argv[4]

output_filename = dataset_name + ".prefix_topk.breakdowns.json"

for i in range(1, 8):
    os.system("../build/benchmark_prefix_topk " + type + " " + k + " ../build/" + dataset_name + ".bin " + str(i) + " " + str(num_queries) + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=" + str(i) + ".shuffled 2>> " + output_filename)
os.system("../build/benchmark_prefix_topk " + type + " " + k + " ../build/" + dataset_name + ".bin 8+ " + str(num_queries) + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=8+.shuffled 2>> " + output_filename)
