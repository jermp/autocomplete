#include "test_common.hpp"

using namespace autocomplete;

uint32_t naive_topk(std::vector<id_type> const& input, range r, uint32_t k,
                    std::vector<id_type>& topk, bool unique = false) {
    uint32_t range_len = r.end - r.begin;
    for (uint32_t i = 0; i != range_len; ++i) {
        topk[i] = input[r.begin + i];
    }
    std::sort(topk.begin(), topk.begin() + range_len);
    uint32_t results = 0;
    if (unique) {
        auto end = std::unique(topk.begin(), topk.begin() + range_len);
        results = std::min<uint32_t>(k, std::distance(topk.begin(), end));
    } else {
        results = std::min<uint32_t>(k, range_len);
    }
    return results;
}

std::vector<range> gen_random_queries(uint32_t num_queries,
                                      uint32_t max_range_len) {
    std::vector<range> queries;
    queries.reserve(num_queries);
    essentials::uniform_int_rng<uint32_t> random(0, max_range_len);
    for (uint32_t i = 0; i != num_queries; ++i) {
        uint32_t x = random.gen();
        uint32_t y = random.gen();
        range r;
        if (y > x) {
            r = {x, y};
        } else {
            r = {y, x};
        }
        queries.push_back(r);
    }
    return queries;
}

TEST_CASE("test unsorted_list on doc_ids") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    static const uint32_t k = 10;
    static_assert(k <= constants::MAX_K, "k must be less than max allowed");
    static const uint32_t num_queries = 5000;

    std::vector<id_type> doc_ids;

    {
        doc_ids.reserve(params.num_completions);
        std::ifstream input(params.collection_basename + ".mapped",
                            std::ios_base::in);
        completion_iterator it(params, input);
        while (input) {
            auto const& record = *it;
            doc_ids.push_back(record.doc_id);
            ++it;
        }
        input.close();

        // {
        //     // must have all ids from 0 to doc_ids.size() - 1
        //     // NOTE: not true if we filter out some strings to be used as
        //     // queries
        //     std::vector<id_type> tmp = doc_ids;
        //     std::sort(tmp.begin(), tmp.end());
        //     for (id_type id = 0; id != doc_ids.size(); ++id) {
        //         REQUIRE_MESSAGE(tmp[id] == id,
        //                         "Error: id " << id << " not found");
        //     }
        // }

        unsorted_list_type list;
        list.build(doc_ids);
        REQUIRE(list.size() == doc_ids.size());
        essentials::save<unsorted_list_type>(list, output_filename);
    }

    {
        unsorted_list_type list;
        essentials::load(list, output_filename);

        std::vector<id_type> topk(constants::MAX_K);
        auto queries = gen_random_queries(num_queries, doc_ids.size());
        std::vector<id_type> expected(params.num_completions);

        for (auto q : queries) {
            uint32_t expected_results = naive_topk(doc_ids, q, k, expected);
            uint32_t results = list.topk(q, k, topk);
            REQUIRE_MESSAGE(expected_results == results,
                            "Error: expected " << expected_results
                                               << " topk elements but got "
                                               << results);
            for (uint32_t i = 0; i != results; ++i) {
                REQUIRE_MESSAGE(topk[i] == expected[i],
                                "Error: expected " << expected[i] << " but got "
                                                   << topk[i]);
            }
        }

        std::remove(output_filename);
    }
}

TEST_CASE("test unsorted_list on minimal doc_ids") {
    char const* output_filename = testing::tmp_filename.c_str();
    parameters params;
    params.collection_basename = testing::test_filename.c_str();
    params.load();

    static const uint32_t k = 10;
    static_assert(k <= constants::MAX_K, "k must be less than max allowed");
    static const uint32_t num_queries = 5000;

    std::vector<id_type> doc_ids;

    {
        doc_ids.reserve(params.num_terms);
        std::ifstream input((params.collection_basename + ".inverted").c_str(),
                            std::ios_base::in);
        id_type first;
        for (uint64_t i = 0; i != params.num_terms; ++i) {
            uint32_t n = 0;
            input >> n;
            input >> first;
            doc_ids.push_back(first);
            for (uint64_t k = 1; k != n; ++k) {
                id_type x;
                input >> x;
                (void)x;  // discard
            }
        }
        input.close();
        REQUIRE(doc_ids.size() == params.num_terms);

        unsorted_list_type list;
        list.build(doc_ids);
        REQUIRE(list.size() == doc_ids.size());
        essentials::save<unsorted_list_type>(list, output_filename);
    }

    {
        unsorted_list_type list;
        essentials::load(list, output_filename);

        std::vector<id_type> topk(constants::MAX_K);
        auto queries = gen_random_queries(num_queries, doc_ids.size());
        constexpr bool unique = true;
        std::vector<id_type> expected(params.num_terms);

        for (auto q : queries) {
            uint32_t expected_results =
                naive_topk(doc_ids, q, k, expected, unique);
            uint32_t results = list.topk(q, k, topk, unique);
            REQUIRE_MESSAGE(expected_results == results,
                            "Error: expected " << expected_results
                                               << " topk elements but got "
                                               << results);
            for (uint32_t i = 0; i != results; ++i) {
                REQUIRE_MESSAGE(topk[i] == expected[i],
                                "Error: expected " << expected[i] << " but got "
                                                   << topk[i]);
            }
        }

        std::remove(output_filename);
    }
}