#include "test_common.hpp"

using namespace autocomplete;

typedef ef_completion_trie completion_trie_type;

TEST_CASE("test completion_trie::is_member()") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    {
        completion_trie_type::builder builder(params);
        completion_trie_type ct;
        builder.build(ct);
        REQUIRE(ct.size() == params.num_completions);
        essentials::save<completion_trie_type>(ct, output_filename);
    }

    {
        completion_trie_type ct;
        essentials::load(ct, output_filename);
        REQUIRE(ct.size() == params.num_completions);
        std::ifstream input(params.collection_basename + ".mapped",
                            std::ios_base::in);
        INFO("testing is_member()");
        completion_iterator it(params, input);
        while (input) {
            auto& record = *it;
            REQUIRE(ct.is_member(record.completion));
            ++it;
        }
        input.close();
        std::remove(output_filename);
    }
}
