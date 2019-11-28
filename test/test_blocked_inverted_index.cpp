#include "test_common.hpp"

using namespace autocomplete;

typedef ef_blocked_inverted_index blocked_inverted_index_type;
typedef ef_inverted_index inverted_index_type;

TEST_CASE("test blocked_inverted_index::intersection_iterator") {
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    inverted_index_type ii;

    {
        inverted_index_type::builder ii_builder(params);
        ii_builder.build(ii);
        REQUIRE(ii.num_docs() == params.universe);
        REQUIRE(ii.num_terms() == params.num_terms);
    }

    {
        static const uint32_t num_queries = 10000;
        static const uint32_t max_num_terms = 3;
        auto queries = testing::gen_random_queries(num_queries, max_num_terms,
                                                   params.num_terms);

        static const std::vector<float> C = {0.0125, 0.025, 0.05, 0.1};
        blocked_inverted_index_type blocked_ii;
        uint64_t total;

        for (auto c : C) {
            total = 0;
            {
                blocked_inverted_index_type::builder blocked_ii_builder(params,
                                                                        c);
                blocked_ii_builder.build(blocked_ii);
            }

            REQUIRE(blocked_ii.num_docs() == params.universe);
            REQUIRE(blocked_ii.num_terms() == params.num_terms);

            for (auto& q : queries) {
                auto ii_it = ii.intersection_iterator(q);
                auto blocked_ii_it =
                    blocked_ii.intersection_iterator(q, {0, 0});

                uint32_t n = 0;
                for (; ii_it.has_next(); ++n, ++ii_it, ++blocked_ii_it) {
                    auto got = *blocked_ii_it;
                    auto expected = *ii_it;
                    REQUIRE_MESSAGE(got == expected, "expected doc_id "
                                                         << expected
                                                         << " but got " << got);
                }
                if (n) total += n;
                REQUIRE(blocked_ii_it.has_next() == false);
            }

            std::cout << total << std::endl;
        }
    }
}
