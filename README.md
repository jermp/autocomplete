Autocomplete
------------

A Query Auto-Completion system based on the paper *[Efficient and Effective Query Auto-Completion](https://dl.acm.org/doi/10.1145/3397271.3401432)*, by Simon Gog, Giulio Ermanno Pibiri, and Rossano Venturini,
published in ACM SIGIR 2020.

Please, cite the paper if you use the data structures from this library.

##### Table of contents
1. [Installation and quick start](#install)
2. [Compiling the code](#compiling)
3. [Input data format](#input)
4. [Running the unit tests](#testing)
5. [Building an index](#building)
6. [Benchmarks](#benchmarks)
7. [Live demo](#demo)

Installation and quick start <a name="install"></a>
------------------

Just run

	bash ./install.sh

from the parent directory. The script builds the code; prepare the test data in the folder `test_data/trec_05_efficiency_queries` for indexing; executes the unit tests.

After that, for having a minimal running example, just run

	bash ./example.sh

and then access the service [from localhost](http://localhost:8000).

### Or you can use a prebuilt Docker image

The following command pulls a prebuilt Docker image and runs it locally.

	docker pull jermp/autocomplete
	docker run -p 8000:8000 -d jermp/autocomplete

And then access the service [from localhost](http://localhost:8000).

Compiling the code <a name="compiling"></a>
------------------

The code has been tested on Linux with `gcc` 7.4.0, 8.3.0, 9.0.0, on Mac OS 10.14 and 12.4 with `clang` 10.0.0 and 13.0.0.

To build the code, [`CMake`](https://cmake.org/) is required.

Clone the repository with

	git clone --recursive https://github.com/jermp/autocomplete.git

If you have cloned the repository without `--recursive`, you will need to perform the following commands before
compiling:

    git submodule init
    git submodule update

To compile the code for a release environment (see file `CMakeLists.txt` for the used compilation flags), it is sufficient to do the following:

    mkdir build
    cd build
    cmake ..
    make

Hint: Use `make -j` to compile the library in parallel using all
available threads.

For the best of performance, we recommend compiling with:

	cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SANITIZERS=Off -DUSE_INTRINSICS=On -DUSE_PDEP=On

For a testing environment, use the following instead:

    mkdir debug_build
    cd debug_build
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSE_SANITIZERS=On
    make

Input data format <a name="input"></a>
-----------------

The input file should list all completions in
*lexicographical* order.
For example, see the the file `test_data/trec_05_efficiency_queries/trec_05_efficiency_queries.completions`.

The first column represent the
ID of the completion; the other columns contain the
tokens separated by white spaces.
(The IDs for the file `trec_05_efficiency_queries.completions` are
fake, i.e., they do not take into account any
particular assignment.)

The script `preprocess.sh` in the directory `test_data` helps
in preparing the data for indexing.
Thus, from within the directory `test_data`, it is sufficient
to do:

	bash preprocess.sh <test_collection> <num_queries>

Therefore, for our example with `trec_05_efficiency_queries`, it would be:

	bash preprocess.sh trec_05_efficiency_queries/trec_05_efficiency_queries.completions 300

The second argument in the example, i.e., 300, represents the
number of completions (per completion size) that are drawn at
random and could be used to query the indexes.

If you run the script, you will get:

- `trec_05_efficiency_queries.completions.dict`: lists all the distinct
tokens in the completions sorted in lexicographical
order.

- `trec_05_efficiency_queries.completions.mapped`: lists all completions
whose tokens have been mapped to integer ids
as assigned by a lexicographically-sorted
string dictionary (that should be built from the
tokens listed in `trec_05_efficiency_queries.completions.dict`).
Each completion terminates with the id `0`.

- `trec_05_efficiency_queries.completions.mapped.stats` contains some
statistics about the datasets, needed to build
the data structures more efficiently.

- `trec05_efficiency_queries.completions.inverted` is the inverted file.

- `trec_05_efficiency_queries.completions.forward` is the forward file. Note that each list is *not* sorted, thus the lists are the same as the ones contained in `trec_05_efficiency_queries.completions.mapped` but sorted in docID order.

Running the unit tests <a name="testing"></a>
-----------

The unit tests are written using [doctest](https://github.com/onqtam/doctest).

After compilation and preparation of the data for indexing (see Section [Input data format](#input)), it is advised
to run the unit tests with:

	make test

Building an index <a name="building"></a>
-----------

After compiling the code, run the program `./build` to build an index. You can specify the type of the index and the name of the file
where the index will be written.

For example, with

	./build ef_type1 ../test_data/trec_05_efficiency_queries/trec_05_efficiency_queries.completions -o trec05.ef_type1.bin

we can build an index of type `ef_type1` from the test file `../test_data/trec_05_efficiency_queries/trec_05_efficiency_queries.completions`, that will be serialized to the file `trec05.ef_type1.bin`.

Possible types are `ef_type1`, `ef_type2`, `ef_type3` and `ef_type4`.

Note: the type `ef_type4` requires an extra parameter
to be specified, `c`. Use for example: `-c 0.0001`.

Benchmarks <a name="benchmarks"></a>
----------

To run the top-k benchmarks in the `/benchmark` directory,
we first need some query logs.
They should have been created already if you have run the
script `preprocess.sh`, otherwise
you can use

	python3 partition_queries_by_length.py trec_05_efficiency_queries/trec_05_efficiency_queries.completions trec_05_efficiency_queries/trec_05_efficiency_queries.completions.queries 300

to partition the input completions by number of query terms
and retain 300 queries at random.
Query files are placed in the output directory
`trec_05_efficiency_queries/trec_05_efficiency_queries.completions.queries`.
(By default, 7 shards will be created: the ones having [1,6] query terms and
the one collecting all completions with *at least* 7 query terms).

Then the command

	./benchmark_topk ef_type1 10 trec05.ef_type1.bin 3 300 0.25 < ../test_data/trec_05_efficiency_queries/trec_05_efficiency_queries.completions.queries/queries.length=3.shuffled

will execute 1000 top-10 queries with 3 terms, from which only 25%
of the prefix of the last token is retained.

We automated the collection of results with the script `script/collected_topk_results_by_varying_percentage.py`.
From within the `/build` directory, run

	python3 ../script/collect_results_by_varying_percentage.py ef_type1 topk trec05.ef_type1.bin ../test_data/trec_05_efficiency_queries/trec_05_efficiency_queries.completions 10 300

To benchmark the dictionaries (Front-Coding and trie), just run the following script from within
the `script` directory:

    bash benchmark_dictionaries.sh

Live demo <a name="demo"></a>
----------

Start the web server with the program `./web_server <port> <index_filename>` and access the demo at
`localhost:<port>`.
