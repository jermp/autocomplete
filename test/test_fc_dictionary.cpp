#include "test_common.hpp"

using namespace autocomplete;

TEST_CASE("test fc_dictionary") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    {
        fc_dictionary_type::builder builder(params);
        fc_dictionary_type dict;
        builder.build(dict);
        essentials::save<fc_dictionary_type>(dict, output_filename);
    }

    {
        fc_dictionary_type dict;
        essentials::load(dict, output_filename);

        // test locate() and extract for all strings
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
        input.close();

        std::vector<uint8_t> decoded(2 * constants::MAX_NUM_CHARS_PER_QUERY);

        for (auto const& t : terms) {
            id_type expected = testing::locate(terms, t);
            id_type got = dict.locate(string_to_byte_range(t));

            REQUIRE_MESSAGE(got == expected, "expected id " << expected
                                                            << ", but got id "
                                                            << got);

            uint8_t string_len = dict.extract(got, decoded.data());
            REQUIRE_MESSAGE(string_len == t.size(),
                            "expected size " << t.size() << ", but got size "
                                             << string_len);

            auto s = reinterpret_cast<char const*>(decoded.data());
            for (uint8_t i = 0; i != string_len; ++i) {
                REQUIRE_MESSAGE(t[i] == s[i], "expected char " << t[i]
                                                               << " but got "
                                                               << s[i]);
            }
        }

        // test locate_prefix() for all strings
        std::string prefix;
        prefix.reserve(256 + 1);
        for (auto const& t : terms) {
            uint32_t n = t.size();
            for (uint32_t prefix_len = 1; prefix_len <= n; ++prefix_len) {
                prefix.clear();
                for (uint32_t i = 0; i != prefix_len; ++i) {
                    prefix.push_back(t[i]);
                }

                range expected = testing::locate_prefix(terms, prefix);
                range got = dict.locate_prefix(string_to_byte_range(prefix));
                REQUIRE_MESSAGE((got.begin == expected.begin and
                                 got.end == expected.end - 1),
                                "Error for prefix '"
                                    << prefix << "' : expected ["
                                    << expected.begin << "," << expected.end - 1
                                    << "] but got [" << got.begin << ","
                                    << got.end << "]");
            }
        }
        std::remove(output_filename);
    }
}
