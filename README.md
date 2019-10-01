Autocomplete
------------

Query autocompletion in C++.

##### Table of contents
1. [Compiling the code](#compiling)
2. [Input data format](#input)
3. [Benchmarks](#benchmarks)

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

Run `benchmark/benchmark_prefix_topk` and `benchmark_conjunctive_topk`.

See the directory `results` for the results on the AOL query log.