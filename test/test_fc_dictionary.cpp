#include <iostream>

#include "types.hpp"

using namespace autocomplete;

term_id_type id(std::vector<std::string> const& terms, std::string const& t) {
    return std::distance(terms.begin(),
                         std::lower_bound(terms.begin(), terms.end(), t));
}

int main(int argc, char** argv) {
    int mandatory = 2;
    if (argc < mandatory) {
        std::cout << argv[0] << " <collection_basename> [-o output_filename]"
                  << std::endl;
        return 1;
    }

    char const* output_filename = nullptr;

    for (int i = mandatory; i != argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            ++i;
            output_filename = argv[i];
        }
    }

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    {
        // build, print and write
        fc_dictionary_B16::builder builder(params);
        fc_dictionary_B16 dict;
        builder.build(dict);
        std::cout << "using " << dict.bytes() << " bytes" << std::endl;

        if (output_filename) {
            // essentials::print_size(dict);
            essentials::logger("saving data structure to disk...");
            essentials::save<fc_dictionary_B16>(dict, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        // load and print
        fc_dictionary_B16 dict;
        essentials::logger("loading data structure from disk...");
        essentials::load(dict, output_filename);
        essentials::logger("DONE");
        // essentials::print_size(dict);
        std::cout << "using " << dict.bytes() << " bytes" << std::endl;

        // test id() for all strings
        std::vector<std::string> terms;
        terms.reserve(params.num_terms);
        std::ifstream input((params.collection_basename + ".dict").c_str(),
                            std::ios_base::in);
        if (!input.good()) {
            throw std::runtime_error("File not found");
        }
        std::string term;
        term.reserve(256 + 1);
        input >> term;
        while (input) {
            terms.push_back(std::move(term));
            input >> term;
        }

        std::cout << "terms.size() " << terms.size() << std::endl;

        for (auto const& t : terms) {
            term_id_type expected = id(terms, t);
            term_id_type got = dict.id(string_to_byte_range(t));

            if (got != expected) {
                std::cout << "Error: expected id " << expected << ","
                          << " but got id " << got << std::endl;
                return 1;
            }

            std::cout << "lexicographic id of '" << t << "' is " << got
                      << std::endl;
        }
    }

    return 0;
}
