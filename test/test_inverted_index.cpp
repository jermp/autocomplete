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

    typedef ef_inverted_index inverted_index_type;

    {
        // build, print and write
        inverted_index_type::builder builder(params);
        inverted_index_type index;
        builder.build(index);
        std::cout << "using " << index.bytes() << " bytes" << std::endl;
        std::cout << "num docs " << index.num_docs() << std::endl;
        std::cout << "num terms " << index.num_terms() << std::endl;

        if (output_filename) {
            essentials::logger("saving data structure to disk...");
            essentials::save<inverted_index_type>(index, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            inverted_index_type index;
            essentials::logger("loading data structure from disk...");
            essentials::load(index, output_filename);
            essentials::logger("DONE");
            std::cout << "using " << index.bytes() << " bytes" << std::endl;
            std::cout << "num docs " << index.num_docs() << std::endl;
            std::cout << "num terms " << index.num_terms() << std::endl;

            std::vector<id_type> intersection(index.num_docs());  // at most
            std::vector<id_type> term_ids;
            term_ids.reserve(2);

            // id_type i = 293;
            // id_type j = 294;
            // id_type i = 899;
            // id_type j = 822;
            id_type i = 2401599 - 1;
            id_type j = 1752198 - 1;
            term_ids.push_back(i + 1);
            term_ids.push_back(j + 1);
            // uint64_t size = index.intersect(term_ids, intersection);

            {
                std::cout << "intersection between " << i << " and " << j
                          << " is: ";
                uint32_t i = 0;
                auto intersec_it = index.intersection_iterator(term_ids);
                while (intersec_it.has_next()) {
                    id_type doc_id = *intersec_it;
                    std::cout << doc_id << " ";
                    ++i;
                    ++intersec_it;
                }
                std::cout << std::endl;
            }

            std::vector<id_type> a;
            {
                auto it = index.iterator(i);
                a.resize(it.size());
                for (uint32_t i = 0; i != a.size(); ++i) {
                    a[i] = it.access(i);
                }
            }

            std::vector<id_type> b;
            {
                auto it = index.iterator(j);
                b.resize(it.size());
                for (uint32_t i = 0; i != b.size(); ++i) {
                    b[i] = it.access(i);
                }
            }

            auto it = std::set_intersection(a.begin(), a.end(), b.begin(),
                                            b.end(), intersection.begin());
            intersection.resize(it - intersection.begin());
            std::cout << "intersection between " << i << " and " << j
                      << " is: ";
            for (auto x : intersection) {
                std::cout << x << " ";
            }
            std::cout << std::endl;

            // for (uint32_t i = 1; i != index.num_terms() + 1; ++i) {
            //     for (uint32_t j = i; j != index.num_terms() + 1; ++j) {
            //         term_ids.clear();
            //         term_ids.push_back(i);
            //         term_ids.push_back(j);
            //         uint64_t size = index.intersect(term_ids, intersection);
            //         std::cout << "size of intersection between " << i << "
            //         and "
            //                   << j << " is " << size << std::endl;
            //     }
            // }
        }
    }

    return 0;
}
