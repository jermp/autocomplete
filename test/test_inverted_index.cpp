#include "test_common.hpp"

using namespace autocomplete;

typedef ef_inverted_index inverted_index_type;

TEST_CASE("test inverted_index::iterator") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    {
        inverted_index_type::builder builder(params);
        inverted_index_type index;
        builder.build(index);
        REQUIRE(index.num_docs() == params.universe);
        REQUIRE(index.num_terms() == params.num_terms);
        essentials::save<inverted_index_type>(index, output_filename);
    }

    {
        inverted_index_type index;
        essentials::load(index, output_filename);
        REQUIRE(index.num_docs() == params.universe);
        REQUIRE(index.num_terms() == params.num_terms);

        std::ifstream input((params.collection_basename + ".inverted").c_str(),
                            std::ios_base::in);
        for (uint64_t i = 0; i != index.num_terms(); ++i) {
            auto it = index.iterator(i);
            uint32_t n = 0;
            input >> n;
            REQUIRE_MESSAGE(n == it.size(), "list has size " << it.size()
                                                             << " instead of "
                                                             << n);
            for (uint64_t k = 0; k != n; ++k, ++it) {
                id_type expected;
                input >> expected;
                auto got = *it;
                REQUIRE_MESSAGE(got == expected,
                                "got " << got << " but expected " << expected);
            }
        }
        input.close();

        std::remove(output_filename);
    }
};

TEST_CASE("test inverted_index::intersection_iterator") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    {
        inverted_index_type::builder builder(params);
        inverted_index_type index;
        builder.build(index);
        REQUIRE(index.num_docs() == params.universe);
        REQUIRE(index.num_terms() == params.num_terms);
        essentials::save<inverted_index_type>(index, output_filename);
    }

    {
        inverted_index_type index;
        essentials::load(index, output_filename);
        REQUIRE(index.num_docs() == params.universe);
        REQUIRE(index.num_terms() == params.num_terms);

        static const uint32_t num_queries = 1000000;
        static const uint32_t max_num_terms = 5;
        auto queries = testing::gen_random_queries(num_queries, max_num_terms,
                                                   index.num_terms());

        std::vector<id_type> first(index.num_docs());
        std::vector<id_type> second(index.num_docs());
        std::vector<id_type> intersection(index.num_docs());

        for (auto const& q : queries) {
            uint32_t first_size = 0;
            uint32_t second_size = 0;
            assert(q.size() >= 2);

            {
                auto it = index.iterator(q[0] - 1);
                first_size = it.size();
                for (uint32_t i = 0; i != first_size; ++i) {
                    first[i] = it.access(i);
                }
            }

            {
                auto it = index.iterator(q[1] - 1);
                second_size = it.size();
                for (uint32_t i = 0; i != second_size; ++i) {
                    second[i] = it.access(i);
                }
            }

            auto end = std::set_intersection(
                first.begin(), first.begin() + first_size, second.begin(),
                second.begin() + second_size, intersection.begin());
            first_size = std::distance(intersection.begin(), end);
            first.swap(intersection);

            for (uint32_t i = 2; i != q.size(); ++i) {
                auto it = index.iterator(q[i] - 1);
                second_size = it.size();
                for (uint32_t i = 0; i != second_size; ++i) {
                    second[i] = it.access(i);
                }
                end = std::set_intersection(
                    first.begin(), first.begin() + first_size, second.begin(),
                    second.begin() + second_size, intersection.begin());
                first_size = std::distance(intersection.begin(), end);
                first.swap(intersection);
            }

            auto it = index.intersection_iterator(q);
            uint32_t n = 0;
            for (; it.has_next(); ++n, ++it) {
                auto doc_id = *it;
                REQUIRE_MESSAGE(
                    doc_id == first[n],
                    "expected doc_id " << first[n] << " but got " << doc_id);
            }
            REQUIRE_MESSAGE(n == first_size, "expected " << first_size
                                                         << " results, but got "
                                                         << n);
        }
        std::remove(output_filename);
    }
}
