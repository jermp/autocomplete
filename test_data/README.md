Input data format
-----------------

The test file `completions` lists all completions in
*lexicographical* order.

The first column represent the
ID of the completion; the other columns contain the
tokens separated by white spaces.

The file `completions.dict` lists all the distinct
tokens in the completions sorted in lexicographical
order.

The file `completions.mapped` lists all completions
whose tokens have been mapped to integer ids
as assigned by a lexicographically-sorted
string dictionary (that should be built from the
tokens listed in `completions.dict`).
Each completion terminates with the id `0`.

The metafile `completions.stats` contains some
statistics about the datasets, needed to build
the data structures more efficiently.

- The script `extract_dict.py` extract the dictionary
from a file listing all completions in textual form.

- The script `map_dataset.py` maps strings to integer ids.

- The script `build_stats.py` calulcates the dataset statistics.