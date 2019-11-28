git submodule init
git submodule update
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SANITIZERS=Off -DUSE_INTRINSICS=On -DUSE_PDEP=On
make
cd ../test_data
bash preprocess.sh trec_05_efficiency_queries/trec_05_efficiency_queries.completions 300
cd ../build
make test
cd ..
