import sys, os

type = sys.argv[1]
exe = sys.argv[2] # prefix_top, conjunctive_topk, topk
dataset_name = sys.argv[3]
k = sys.argv[4]
num_queries = sys.argv[5]
collect_breakdowns = int(sys.argv[6]) # 0 or 1

breakdown = ""
if collect_breakdowns != 0:
    breakdown = "--breakdown"

output_filename = dataset_name + "." + exe + ".timings.json"

for i in range(1, 8):
    os.system("../build/benchmark_" + exe + " " + type + " " + k + " ../build/" + dataset_name + ".bin " + str(i) + " " + str(num_queries) + " " + breakdown + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=" + str(i) + ".shuffled 2>> " + output_filename)
os.system("../build/benchmark_" + exe + " " + type + " " + k + " ../build/" + dataset_name + ".bin 8+ " + str(num_queries) + " " + breakdown + " < ../test_data/" + dataset_name + "/" + dataset_name + ".completions.length=8+.shuffled 2>> " + output_filename)
