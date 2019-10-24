#include <iostream>

#include "types.hpp"
#include "../benchmark/benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void check_topk(char const* binary_filename1, char const* binary_filename2,
                uint32_t k, uint32_t max_num_queries, float keep) {
    Index index1;
    ef_autocomplete_type1 index2;
    essentials::load(index1, binary_filename1);
    essentials::load(index2, binary_filename2);
    std::vector<std::string> queries;
    load_queries(queries, max_num_queries, keep, std::cin);
    for (auto const& query : queries) {
        size_t n1 = index1.topk(query, k).size();
        size_t n2 = index2.topk(query, k).size();
        if (n1 != n2) {
            std::cout << query << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    int mandatory = 6;
    if (argc < mandatory + 1) {
        std::cout << argv[0]
                  << " <type> <k> <binary_filename1> "
                     "<binary_filename2> "
                     "<max_num_queries> <percentage> < queries"
                  << std::endl;
        std::cout << "<percentage> is a float in [0,1] and specifies how much "
                     "we keep of the last token in a query "
                  << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    uint32_t k = std::atoi(argv[2]);
    char const* binary_filename1 = argv[3];
    char const* binary_filename2 = argv[4];
    uint32_t max_num_queries = std::atoi(argv[5]);
    float keep = std::atof(argv[6]);

    if (type == "ef_type1") {
        check_topk<ef_autocomplete_type1>(binary_filename1, binary_filename2, k,
                                          max_num_queries, keep);
    } else if (type == "ef_type2") {
        check_topk<ef_autocomplete_type2>(binary_filename1, binary_filename2, k,
                                          max_num_queries, keep);
    } else if (type == "ef_type3") {
        check_topk<ef_autocomplete_type3>(binary_filename1, binary_filename2, k,
                                          max_num_queries, keep);
    } else if (type == "ef_type4") {
        check_topk<ef_autocomplete_type4>(binary_filename1, binary_filename2, k,
                                          max_num_queries, keep);
    } else {
        return 1;
    }

    return 0;
}