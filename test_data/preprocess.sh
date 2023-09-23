#!/bin/bash

echo $1 # input filename
echo $2 # number of queries for each size
python3 extract_dict.py $1
python3 map_dataset.py $1
python3 build_stats.py $1.mapped
python3 build_inverted_and_forward.py $1
python3 partition_queries_by_length.py $1 $1.queries $2
