#include <iostream>
#include <functional>

#include "types.hpp"

using namespace autocomplete;

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
        // build and write

        // std::vector<uint32_t> doc_ids = {23, 2, 4,  0,  88, 23, 2, 4,  55, 3,
        //                            7,  6, 90, 34, 2,  3,  1, 12, 23};

        std::vector<uint32_t> doc_ids;
        doc_ids.reserve(params.num_completions);
        std::ifstream input(params.collection_basename + ".mapped",
                            std::ios_base::in);
        if (!input.good()) {
            throw std::runtime_error("File not found");
        }
        completion_iterator it(params, input);
        while (input) {
            completion const& c = *it;
            doc_ids.push_back(c.doc_id);
            ++it;
        }
        input.close();

        cartesian_tree rmq;
        rmq.build(doc_ids, std::less<uint32_t>());
        assert(rmq.size() == doc_ids.size());
        std::cout << "using " << rmq.bytes() << " bytes" << std::endl;

        if (output_filename) {
            // essentials::print_size(rmq);
            essentials::logger("saving data structure to disk...");
            essentials::save<cartesian_tree>(rmq, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        // load and print
        cartesian_tree rmq;
        essentials::logger("loading data structure from disk...");
        essentials::load(rmq, output_filename);
        essentials::logger("DONE");

        std::cout << "using " << rmq.bytes() << " bytes" << std::endl;

        for (size_t i = 0; i != rmq.size(); ++i) {
            for (size_t j = i; j != rmq.size(); ++j) {
                std::cout << "rmq[" << i << "," << j << "] = " << rmq.rmq(i, j)
                          << std::endl;
            }
        }
    }

    return 0;
}
