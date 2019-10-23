git submodule init
git submodule update
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SANITIZERS=Off -DUSE_INTRINSICS=On -DUSE_PDEP=On
make
cd ../test_data
./preprocess.sh
cd ../build
make test
cd ..
