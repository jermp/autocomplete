import sys, os

dataset_name = sys.argv[1] # e.g., aol
types = ["ef_type1", "ef_type2", "ef_type3", "ef_type4"]
for t in types:
    os.system("./build " + t + " ../test_data/" + dataset_name + "/" + dataset_name + ".completions -o " + t + "." + dataset_name + ".bin -c 0.0001")