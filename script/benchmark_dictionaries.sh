cd ../test_data
bash preprocess.sh aol/aol.completions 100000
cd ../build
python ../script/collect_locate_prefix_results_by_varying_percentage.py fc ../test_data/aol/aol.completions 100000
python ../script/collect_locate_prefix_results_by_varying_percentage.py trie ../test_data/aol/aol.completions 100000
./benchmark_fc_dictionary ../test_data/aol/aol.completions 100000 < ../test_data/aol/aol.completions.queries/queries.length=1 > ../test_data/aol/aol.completions.dictionary_benchmark.txt
cd ../script