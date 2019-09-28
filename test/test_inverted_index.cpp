#include <iostream>

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
        // build, print and write
        uncompressed_inverted_index::builder builder(params);
        uncompressed_inverted_index ii;
        builder.build(ii);
        std::cout << "using " << ii.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << ii.num_docs() << std::endl;
        std::cout << "num terms " << ii.num_terms() << std::endl;

        if (output_filename) {
            essentials::logger("saving data structure to disk...");
            essentials::save<uncompressed_inverted_index>(ii, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        // load and print
        uncompressed_inverted_index ii;
        essentials::logger("loading data structure from disk...");
        essentials::load(ii, output_filename);
        essentials::logger("DONE");
        std::cout << "using " << ii.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << ii.num_docs() << std::endl;
        std::cout << "num terms " << ii.num_terms() << std::endl;

        std::vector<id_type> intersection(ii.num_docs());  // at most
        std::vector<id_type> term_ids;
        term_ids.reserve(2);

        // id_type i = 293;
        // id_type j = 294;
        // id_type i = 899;
        // id_type j = 822;
        id_type i = 14145;
        id_type j = 5430;
        term_ids.push_back(i);
        term_ids.push_back(j);
        uint64_t size = ii.intersect(term_ids, intersection);

        std::cout << "size of intersection between " << i << " and " << j
                  << " is " << size << ": ";
        for (uint32_t i = 0; i != size; ++i) {
            std::cout << intersection[i] << " ";
        }
        std::cout << std::endl;

        std::vector<id_type> a;
        {
            auto it = ii.iterator(i);
            a.resize(it.size());
            for (uint32_t i = 0; i != a.size(); ++i) {
                a[i] = it.access(i);
            }
        }

        std::vector<id_type> b;
        {
            auto it = ii.iterator(j);
            b.resize(it.size());
            for (uint32_t i = 0; i != b.size(); ++i) {
                b[i] = it.access(i);
            }
        }

        auto it = std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                                        intersection.begin());
        intersection.resize(it - intersection.begin());
        std::cout << "size of intersection between " << i << " and " << j
                  << " is " << intersection.size() << ": ";
        for (auto x : intersection) {
            std::cout << x << " ";
        }
        std::cout << std::endl;

        // for (uint32_t i = 1; i != ii.num_terms() + 1; ++i) {
        //     for (uint32_t j = i; j != ii.num_terms() + 1; ++j) {
        //         term_ids.clear();
        //         term_ids.push_back(i);
        //         term_ids.push_back(j);
        //         uint64_t size = ii.intersect(term_ids, intersection);
        //         std::cout << "size of intersection between " << i << " and "
        //                   << j << " is " << size << std::endl;
        //     }
        // }
    }

    return 0;
}
