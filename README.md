Autocomplete
------------

Query autocompletion in C++.

##### Table of contents
1. [Description](#descr)
2. [Compiling the code](#compiling)
3. [Input data format](#input)
4. [Benchmarks](#benchmarks)
5. [Live demo](#demo)

Description <a name="descr"></a>
-----------

We designed two solutions (`autocomplete.hpp` and `autocomplete2.hpp`).
The second solution avoids storing the forward index of the first solution.

Both solution build on two steps: (1) a prefix search (`prefix_topk`) and (2) a conjunctive search (`conjunctive_topk`).

Recall that each completion has an associated integer identifier (henceforth, called docID), assigned in *decreasing* score order.

#### 1. Prefix search

This step returns the top-k completions that are prefixed by the terms in the query.
For this purposes, we build a dictionary storing all completions seen as (multi-) sets of termIDs.
Solution 1 uses an integer trie data structure (`completion_trie.hpp`);
Solution 2 uses Front Coding (`integer_fc_dictionary.hpp`).
We also materialize the list L of docIDs sorted by the lexicographical order of the completions (`unsorted_list.hpp`).

During a search, we first map the query terms to their lexicographic IDs by using a string dictionary (implemented as a 2-level index with Front Coding -- `fc_dictionary.hpp`). Then, we search the mapped query, say Q, into the completion trie to obtain the lexicographic range [l,r] of all completions that are children of Q. Then we need to identify the top-k docIDs from L[l,r]. Since the range [l,r] can be very large, we use a RMQ data structure built on L.

Having retrieved a list of (at most) k docIDs, we then:

1. Solution 1: use a forward index (`forward_index.hpp`) to materialize the identified completions into a string pool (`scored_string_pool.hpp`).
The forward index stores the sorted (multi-) set of the termIDs of each completion, plus also the permutation of such termIDs in order to restore the original completion. The sets are stored in increasing-docID order.
Specifically, we use the forward index to obtain the (permuted) set
of termIDs and the string dictionary to extract the strings.

2. Solution 2: use a map from docIDs to lexicographic IDs. For every top-k docID, we extract the corresponding completion from the FC-based dictionary.

#### 2. Conjunctive search

This step returns the top-k completions using an inverted index (`inverted_index.hpp`).
For this purpose, let us consider a query Q[1,m] as tokenized into m terms (the last one possibly not completed).
In this case we want to return the top-k (smallest) docIDs belonging
to the intersection between the posting lists of the first m-1 terms
and the union between all the postings lists of the terms that are
prefixed by Q[m].

To do so, we could trivially materialize the union and then proceed
with the intersection.
The clear problem with this approach is that the number of terms that are prefixed by Q[m] can be very large. Therefore iterating over the union can be overkilling.

To solve this problem, we first obtain the lexicographic range of Q[m] by the string dictionary, say [l,r].
We then iterate over the intersection of the first m-1 terms' posting lists and for each docID x we check whether the range [l,r] intersect the forward list of x. This check is done with the forward index.
If the check succeeds, then x is among the top-k documents.
We keep iterating over the intersection and checking the forward lists until we have k completions or we touch every docID in the intersection.

There is a special case for the case m = 1. In this case, we have no term before the last (only) one, thus we would check *all* forward lists for the range [l,r]. This is too expensive.
Therefore, we use another RMQ data structure, built on the list, say M, of all the first (i.e., *minimal*) docIDs of the posting lists (think of it as the "first" column of the inverted index).
A recursive heap-based algorithm is used to produce the smallest docIDs in M[l,r] using the RMQ data structure.

The final string extraction step is identical to that of the
prefix search.

Compiling the code <a name="compiling"></a>
------------------

The code is tested on Linux with `gcc` 7.4.0 and on Mac 10.14 with `clang` 10.0.0.
To build the code, [`CMake`](https://cmake.org/) is required.

Clone the repository with

	$ git clone --recursive https://github.com/jermp/autocomplete.git

If you have cloned the repository without `--recursive`, you will need to perform the following commands before
compiling:

    $ git submodule init
    $ git submodule update

To compile the code for a release environment (see file `CMakeLists.txt` for the used compilation flags), it is sufficient to do the following:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

Hint: Use `make -j4` to compile the library in parallel using, e.g., 4 jobs.

For the best of performance, we recommend compiling with:

	$ `cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SANITIZERS=Off -DUSE_INTRINSICS=On -DUSE_PDEP=On`

For a testing environment, use the following instead:

    $ mkdir debug_build
    $ cd debug_build
    $ cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSE_SANITIZERS=On
    $ make
    
Input data format <a name="input"></a>
-----------------

The input file should list all completions in
*lexicographical* order.
For example, see the the file `test_data/trec05_efficiency_queries/trec05_efficiency_queries.completions`.

The first column represent the
ID of the completion; the other columns contain the
tokens separated by white spaces.
(The IDs for the file `trec05_efficiency_queries.completions` are
fake, i.e., they do not take into account any
particular assignment.)

The scripts in the directory `test_data` help in
preparing the datasets for indexing:

1. The command
	
		$ extract_dict.py trec05_efficiency_queries/trec05_efficiency_queries.completions
	
	extract the dictionary
from a file listing all completions in textual form.

2. The command

		$ python map_dataset.py trec05_efficiency_queries/trec05_efficiency_queries.completions
		
	maps strings to integer ids.

3. The command

		$ python build_stats.py trec05_efficiency_queries/trec05_efficiency_queries.completions.mapped
		
	calulcates the dataset statistics.

4. The command

		$ python build_inverted_and_forward.py trec05_efficiency_queries/trec05_efficiency_queries.completions
		
	builds the inverted and forward files.

If you run the scripts in the reported order, you will get:

- `trec05_efficiency_queries.completions.dict`: lists all the distinct
tokens in the completions sorted in lexicographical
order.

- `trec05_efficiency_queries.completions.mapped`: lists all completions
whose tokens have been mapped to integer ids
as assigned by a lexicographically-sorted
string dictionary (that should be built from the
tokens listed in `trec05_efficiency_queries.completions.dict`).
Each completion terminates with the id `0`.

- `trec05_efficiency_queries.completions.mapped.stats` contains some
statistics about the datasets, needed to build
the data structures more efficiently.

- `trec05_efficiency_queries.completions.inverted` is the inverted file.

- `trec05_efficiency_queries.completions.forward` is the forward file. Note that each list is *not* sorted, thus the lists are the same as the ones contained in `trec05_efficiency_queries.completions.mapped` but sorted in docID order.

Benchmarks <a name="benchmarks"></a>
----------

Run `benchmark/benchmark_prefix_topk` and `benchmark/benchmark_conjunctive_topk`.

See the directory `results` for the results on the AOL and MSN query log.

Live demo <a name="demo"></a>
----------

Start the web server with the program `./web_server <port> <index_filename>` and access the demo at
`localhost:<port>`.