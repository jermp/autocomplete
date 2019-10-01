Test machine
------------

4 Intel i7-7700 cores (@3.6 GHz); 64 GB of RAM DDR3 (@2.133 GHz); running Linux 4.4.0 (64 bits); 32K for both instruction and data L1 cache; 256K for L2 cache; 8192K for L3 cache.

Compiler
--------

gcc 7.4.0

`cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SANITIZERS=OFF -DUSE_INTRINSICS=ON -DUSE_PDEP=ON`


Experiments
-----------

- The file `space.md` reports the space breakdowns.
- The file `prefix_topk.md` reports the timing breakdowns for the prefix_topk step by varying the number of query terms.
- The file `conjunctive_topk.md` reports the timing breakdowns for the conjunctive_topk step by varying the number of query terms.
- The file `fc_dictionary.md` reports on the dictionary benchmark.