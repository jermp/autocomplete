#include <iostream>

#include "types.hpp"
#include "statistics.hpp"

using namespace autocomplete;

template <typename Index>
void print_stats(char const* index_filename) {
    Index index;
    essentials::load(index, index_filename);
    index.print_stats();
}

int main(int argc, char** argv) {
    int mandatory = 2;
    if (argc < mandatory + 1) {
        std::cout << argv[0] << " <type> <index_filename>" << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    char const* index_filename = argv[2];

    if (type == "ef_type1") {
        print_stats<ef_autocomplete_type1>(index_filename);
    } else if (type == "ef_type2") {
        print_stats<ef_autocomplete_type2>(index_filename);
    } else if (type == "ef_type3") {
        print_stats<ef_autocomplete_type3>(index_filename);
    } else if (type == "ef_type4") {
        print_stats<ef_autocomplete_type4>(index_filename);
    } else {
        return 1;
    }

    return 0;
}