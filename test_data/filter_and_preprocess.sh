#!/bin/bash

echo $1 # input filename

# number of completions to exclude per completion size,
# e.g., if it is 100, then at most 7 x 100 completions are filtered out
echo $2

python partition_queries_by_length.py $1 $1.filtered.queries $2
python filter_dataset.py $1 $1.filtered.queries
python extract_dict.py $1.filtered
python map_dataset.py $1.filtered
python build_stats.py $1.filtered.mapped
python build_inverted_and_forward.py $1.filtered
