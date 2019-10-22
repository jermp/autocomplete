#include "test_common.hpp"

using namespace autocomplete;

std::vector<id_type> naive_topk(std::vector<id_type> const& input, range r,
                                uint32_t k) {
    uint32_t range_len = r.end - r.begin;
    std::vector<id_type> topk(range_len);
    for (uint32_t i = 0; i != range_len; ++i) {
        topk[i] = input[r.begin + i];
    }
    std::sort(topk.begin(), topk.begin() + range_len);
    topk.resize(std::min<uint32_t>(k, range_len));
    return topk;
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

TEST_CASE("test unsorted_list") {
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

        {
            // must have all ids from 0 to doc_ids.size() - 1
            std::vector<id_type> tmp = doc_ids;
            std::sort(tmp.begin(), tmp.end());
            for (id_type id = 0; id != doc_ids.size(); ++id) {
                REQUIRE_MESSAGE(tmp[id] == id,
                                "Error: id " << id << " not found");
            }
        }

        succinct_rmq list;
        list.build(doc_ids);
        REQUIRE(list.size() == doc_ids.size());

        essentials::save<succinct_rmq>(list, output_filename);
    }

    {
        succinct_rmq list;
        essentials::load(list, output_filename);

        std::vector<id_type> topk(constants::MAX_K);
        auto queries = gen_random_queries(num_queries, doc_ids.size());

        for (auto q : queries) {
            auto expected = naive_topk(doc_ids, q, k);
            uint32_t results = list.topk(q, k, topk);
            REQUIRE_MESSAGE(expected.size() == results,
                            "Error: expected " << expected.size()
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
