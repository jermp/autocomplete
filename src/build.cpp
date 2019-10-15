#include <iostream>

#include "types.hpp"
#include "statistics.hpp"

using namespace autocomplete;

template <typename Index>
void build(parameters const& params, char const* output_filename) {
    Index index(params);
    index.print_stats();
    if (output_filename) {
        essentials::logger("saving data structure to disk...");
        essentials::save<Index>(index, output_filename);
        essentials::logger("DONE");
    }
}

void build_type4(parameters const& params, const float c,
                 char const* output_filename) {
    uncompressed_autocomplete_type4 index(params, c);
    index.print_stats();
    if (output_filename) {
        essentials::logger("saving data structure to disk...");
        essentials::save<uncompressed_autocomplete_type4>(index,
                                                          output_filename);
        essentials::logger("DONE");
    }
}

int main(int argc, char** argv) {
    int mandatory = 2;
    if (argc < mandatory + 1) {
        std::cout << argv[0]
                  << " <type> <collection_basename> [-o output_filename] [-c c]"
                  << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    parameters params;
    params.collection_basename = argv[2];
    params.load();

    char const* output_filename = nullptr;
    float c = 0.0;

    for (int i = mandatory; i != argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            ++i;
            output_filename = argv[i];
        } else if (std::string(argv[i]) == "-c") {
            ++i;
            c = std::stof(argv[i]);
        }
    }

    if (type == "ef_type1") {
        build<ef_autocomplete_type>(params, output_filename);
    } else if (type == "ef_type2") {
        build<ef_autocomplete_type2>(params, output_filename);
    } else if (type == "type1") {
        build<uncompressed_autocomplete_type>(params, output_filename);
    } else if (type == "type2") {
        build<uncompressed_autocomplete_type2>(params, output_filename);
    } else if (type == "type3") {
        build<uncompressed_autocomplete_type3>(params, output_filename);
    } else if (type == "type4") {
        if (c == 0.0) {
            std::cerr << "c must be greater than 0.0" << std::endl;
            return 1;
        }
        build_type4(params, c, output_filename);
    } else {
        return 1;
    }

    return 0;
}