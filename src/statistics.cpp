#include <iostream>

#include "types.hpp"
#include "statistics.hpp"
#include "../external/cmd_line_parser/include/parser.hpp"

using namespace autocomplete;

template <typename Index>
void print_stats(std::string const& index_filename) {
    Index index;
    essentials::load(index, index_filename.c_str());
    index.print_stats();
}

int main(int argc, char** argv) {
    cmd_line_parser::parser parser(argc, argv);
    parser.add("type", "Index type.");
    parser.add("index_filename", "Index filename.");
    if (!parser.parse()) return 1;

    auto type = parser.get<std::string>("type");
    auto index_filename = parser.get<std::string>("index_filename");

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