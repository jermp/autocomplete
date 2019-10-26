#include <iostream>

#include "types.hpp"
#include "statistics.hpp"
#include "../external/cmd_line_parser/include/parser.hpp"

using namespace autocomplete;

template <typename Index>
void build(parameters const& params, std::string const& output_filename) {
    Index index(params);
    index.print_stats();
    if (output_filename != "") {
        essentials::logger("saving data structure to disk...");
        essentials::save<Index>(index, output_filename.c_str());
        essentials::logger("DONE");
    }
}

void build_type4(parameters const& params, const float c,
                 std::string const& output_filename) {
    ef_autocomplete_type4 index(params, c);
    index.print_stats();
    if (output_filename != "") {
        essentials::logger("saving data structure to disk...");
        essentials::save<ef_autocomplete_type4>(index, output_filename.c_str());
        essentials::logger("DONE");
    }
}

int main(int argc, char** argv) {
    cmd_line_parser::parser parser(argc, argv);
    parser.add("type", "Index type.");
    parser.add("collection_basename", "Collection basename.");
    parser.add("output_filename", "Output filename.", "-o", false);
    parser.add(
        "c",
        "Value for Bast and Weber's technique: c must be a float in (0,1].",
        "-c", false);
    if (!parser.parse()) return 1;

    auto type = parser.get<std::string>("type");
    parameters params;
    params.collection_basename = parser.get<std::string>("collection_basename");
    params.load();
    auto output_filename = parser.get<std::string>("output_filename");

    if (type == "ef_type1") {
        build<ef_autocomplete_type1>(params, output_filename);
    } else if (type == "ef_type2") {
        build<ef_autocomplete_type2>(params, output_filename);
    } else if (type == "ef_type3") {
        build<ef_autocomplete_type3>(params, output_filename);
    } else if (type == "ef_type4") {
        auto c = parser.get<float>("c");
        build_type4(params, c, output_filename);
    } else {
        return 1;
    }

    return 0;
}