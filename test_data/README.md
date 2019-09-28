Input data format
-----------------

The input file should list all completions in
*lexicographical* order, e.g., see the the file `trec05_efficiency_queries`.

The first column represent the
ID of the completion; the other columns contain the
tokens separated by white spaces.
(The IDs for the file `trec05_efficiency_queries` are
fake, i.e., they do not take into account any
particular assignment.)

- The script `extract_dict.py` extract the dictionary
from a file listing all completions in textual form.

- The script `map_dataset.py` maps strings to integer ids.

- The script `build_stats.py` calulcates the dataset statistics.

- The script `build_inverted_and_forward.py` builds the inverted and forward files.

If you run such scripts on the test file `trec05_efficiency_queries`, you will get:

- `trec05_efficiency_queries.dict`: lists all the distinct
tokens in the completions sorted in lexicographical
order.

- `trec05_efficiency_queries.mapped`: lists all completions
whose tokens have been mapped to integer ids
as assigned by a lexicographically-sorted
string dictionary (that should be built from the
tokens listed in `trec05_efficiency_queries.dict`).
Each completion terminates with the id `0`.

- `trec05_efficiency_queries.mapped.stats` contains some
statistics about the datasets, needed to build
the data structures more efficiently.

- `trec05_efficiency_queries.inverted` is the inverted file.

- `trec05_efficiency_queries.forward` is the forward file. Note that each list is *not* sorted, thus the lists are the same as the ones contained in `trec05_efficiency_queries.mapped` but sorted in docID order.
