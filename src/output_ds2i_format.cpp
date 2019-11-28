#include <iostream>
#include <fstream>

#include "util.hpp"
#include "parameters.hpp"

using namespace autocomplete;

int main(int argc, char** argv) {
    int mandatory = 1;
    if (argc < mandatory + 1) {
        std::cout << argv[0] << " <collection_basename>" << std::endl;
        return 1;
    }

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    std::ofstream docs((params.collection_basename + ".docs").c_str(),
                       std::ios_base::out | std::ios_base::binary);
    std::ofstream freqs((params.collection_basename + ".freqs").c_str(),
                        std::ios_base::out | std::ios_base::binary);

    std::ifstream input((params.collection_basename + ".inverted").c_str(),
                        std::ios_base::in);

    {  // write ds2i header
        uint32_t n = 1;
        uint32_t universe = params.universe;
        docs.write(reinterpret_cast<const char*>(&n), sizeof(uint32_t));
        docs.write(reinterpret_cast<const char*>(&universe), sizeof(uint32_t));
    }

    uint64_t integers = 0;
    std::vector<uint32_t> docs_list;
    std::vector<uint32_t> freqs_list;
    for (uint64_t i = 0; i != params.num_terms; ++i) {
        docs_list.clear();
        freqs_list.clear();
        uint32_t n = 0;
        input >> n;
        docs_list.reserve(n);
        freqs_list.reserve(n);
        integers += n;
        for (uint64_t k = 0; k != n; ++k) {
            id_type x;
            input >> x;
            docs_list.push_back(x);
            freqs_list.push_back(1);  // fake
        }
        docs.write(reinterpret_cast<const char*>(&n), sizeof(uint32_t));
        freqs.write(reinterpret_cast<const char*>(&n), sizeof(uint32_t));
        docs.write(reinterpret_cast<const char*>(docs_list.data()),
                   n * sizeof(uint32_t));
        freqs.write(reinterpret_cast<const char*>(freqs_list.data()),
                    n * sizeof(uint32_t));
    }
    input.close();
    docs.close();
    freqs.close();

    std::cout << "written " << integers << " integers" << std::endl;

    return 0;
}