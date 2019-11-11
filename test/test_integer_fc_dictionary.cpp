#include "test_common.hpp"

using namespace autocomplete;

TEST_CASE("test integer_fc_dictionary") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    {
        integer_fc_dictionary_type::builder builder(params);
        integer_fc_dictionary_type dict;
        builder.build(dict);
        essentials::save<integer_fc_dictionary_type>(dict, output_filename);
    }

    {
        integer_fc_dictionary_type dict;
        essentials::load(dict, output_filename);

        {
            std::ifstream input(
                (params.collection_basename + ".mapped").c_str(),
                std::ios_base::in);
            completion_iterator it(params, input);

            completion_type decoded(2 * constants::MAX_NUM_TERMS_PER_QUERY);
            for (id_type id = 0; id != params.num_completions; ++id, ++it) {
                auto const& expected = (*it).completion;
                REQUIRE(expected.size() > 0);
                uint8_t size = dict.extract(id, decoded);

                REQUIRE_MESSAGE(expected.size() - 1 == size,
                                "Error in decoding the "
                                    << id << "-th string: expected size "
                                    << expected.size() - 1 << ","
                                    << " but got size " << int(size));

                for (uint8_t i = 0; i != size; ++i) {
                    REQUIRE_MESSAGE(decoded[i] == expected[i],
                                    "Error in decoding the "
                                        << id << "-th string: expected "
                                        << expected[i] << ","
                                        << " but got " << decoded[i]
                                        << " at position " << int(i));
                }

                id_type got_id =
                    dict.locate({decoded.data(), decoded.data() + size});
                REQUIRE(got_id != global::invalid_term_id);
                REQUIRE_MESSAGE(got_id == id, "Error in locating the "
                                                  << id
                                                  << "-th string: expected id "
                                                  << id << ","
                                                  << " but got id " << got_id);
            }

            input.close();
        }
        std::remove(output_filename);
    }
}
