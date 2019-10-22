#!/bin/bash

collections=`find . | grep "\\.completions$"`

for collection in $collections; do
    echo $collection
    python extract_dict.py $collection
    python map_dataset.py $collection
    python build_stats.py $collection.mapped
    python build_inverted_and_forward.py $collection
done
