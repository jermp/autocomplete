#!/bin/bash

echo $1 # input filename
echo $2 # number of queries for each size
python extract_dict.py $1
python map_dataset.py $1
python build_stats.py $1.mapped
python build_inverted_and_forward.py $1
python partition_queries_by_length.py $1 $1.queries $2
