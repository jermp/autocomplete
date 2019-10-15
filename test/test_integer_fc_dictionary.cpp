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
        integer_fc_dictionary_type::builder builder(params);
        integer_fc_dictionary_type dict;
        builder.build(dict);
        std::cout << "using " << dict.bytes() << " bytes" << std::endl;

        if (output_filename) {
            // essentials::print_size(dict);
            essentials::logger("saving data structure to disk...");
            essentials::save<integer_fc_dictionary_type>(dict, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            integer_fc_dictionary_type dict;
            essentials::logger("loading data structure from disk...");
            essentials::load(dict, output_filename);
            essentials::logger("DONE");
            std::cout << "using " << dict.bytes() << " bytes" << std::endl;

            {
                essentials::logger("testing extract() and locate()...");
                std::ifstream input(
                    (params.collection_basename + ".mapped").c_str(),
                    std::ios_base::in);
                completion_iterator it(params, input);

                completion_type decoded(2 * constants::MAX_NUM_TERMS_PER_QUERY);
                for (id_type id = 0; id != params.num_completions; ++id, ++it) {
                    auto const& expected = (*it).completion;
                    assert(expected.size() > 0);
                    uint8_t size = dict.extract(id, decoded);
                    if (expected.size() - 1 != size) {
                        std::cout << "Error in decoding the " << id
                                  << "-th string: expected size "
                                  << expected.size() - 1 << ","
                                  << " but got size " << int(size) << std::endl;
                        return 1;
                    }

                    for (uint8_t i = 0; i != size; ++i) {
                        if (decoded[i] != expected[i]) {
                            std::cout
                                << "Error in decoding the " << id
                                << "-th string: expected " << expected[i] << ","
                                << " but got " << decoded[i] << " at position "
                                << int(i) << std::endl;
                            return 1;
                        }
                    }

                    id_type got_id =
                        dict.locate({decoded.data(), decoded.data() + size});
                    if (got_id != id) {
                        std::cout << "Error in locating the " << id
                                  << "-th string: expected id " << id << ","
                                  << " but got id " << got_id << std::endl;
                        return 1;
                    }
                }

                input.close();
                essentials::logger("it's all good");
            }

            // {
            //     uint64_completion_trie::builder builder(params);
            //     uint64_completion_trie ct;
            //     builder.build(ct);
            //     std::cout << "using " << ct.bytes() << " bytes" << std::endl;

            //     essentials::logger("testing locate_prefix()...");

            //     std::ifstream input(
            //         (params.collection_basename + ".mapped").c_str(),
            //         std::ios_base::in);
            //     completion_iterator it(params, input);

            //     uint32_t num_checks =
            //         std::min<uint32_t>(params.num_completions, 30000);

            //     completion_type prefix;
            //     for (uint32_t i = 0; i != num_checks; ++i, ++it) {
            //         auto const& expected = (*it).completion;
            //         assert(expected.size() > 0);

            //         for (uint32_t prefix_len = 1;
            //              prefix_len <= expected.size() - 1; ++prefix_len) {
            //             prefix.clear();
            //             for (uint32_t i = 0; i != prefix_len; ++i) {
            //                 prefix.push_back(expected[i]);
            //             }

            //             range expected = ct.locate_prefix(prefix);
            //             range got = dict.locate_prefix(
            //                 completion_to_uint32_range(prefix));

            //             if ((got.begin != expected.begin) or
            //                 (got.end != expected.end - 1)) {
            //                 std::cout << "Error for prefix ";
            //                 print_completion(prefix);
            //                 std::cout << ": expected [" << expected.begin <<
            //                 ","
            //                           << expected.end - 1 << "] but got ["
            //                           << got.begin << "," << got.end << "]"
            //                           << std::endl;
            //                 return 1;
            //             }

            //             // std::cout << "prefix range of ";
            //             // print_completion(prefix);
            //             // std::cout << " is [" << got.begin << "," <<
            //             got.end
            //             //           << "]" << std::endl;
            //         }
            //     }

            //     input.close();
            //     essentials::logger("it's all good");
            // }
        }
    }

    return 0;
}
