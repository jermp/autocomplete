cd build
./build ef_type1 ../test_data/trec_05_efficiency_queries/trec_05_efficiency_queries.completions -o trec_05.ef_type1.bin
./web_server 8000 trec_05.ef_type1.bin