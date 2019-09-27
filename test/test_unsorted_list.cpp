#include <iostream>
#include <functional>

#include "types.hpp"

using namespace autocomplete;

static const uint32_t max_k = 15;
static const uint32_t k = 10;
static_assert(k <= max_k, "k must be less than max allowed");
static const uint32_t num_queries = 10000;

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

    std::vector<id_type> doc_ids;

    {
        // build and write
        doc_ids.reserve(params.num_completions);
        std::ifstream input(params.collection_basename + ".mapped",
                            std::ios_base::in);
        if (!input.good()) {
            throw std::runtime_error("File not found");
        }
        completion_iterator it(params, input);
        while (input) {
            completion const& c = *it;
            doc_ids.push_back(c.doc_id);
            ++it;
        }
        input.close();

        {
            // must have all ids from 0 to doc_ids.size() - 1
            std::vector<id_type> tmp = doc_ids;
            std::sort(tmp.begin(), tmp.end());
            for (id_type id = 0; id != doc_ids.size(); ++id) {
                if (tmp[id] != id) {
                    std::cout << "Error: id " << id << " not found"
                              << std::endl;
                    return 1;
                }
            }
        }

        unsorted_list_succinct_rmq list;
        list.build(doc_ids);
        assert(list.size() == doc_ids.size());
        std::cout << "using " << list.bytes() << " bytes" << std::endl;

        if (output_filename) {
            // essentials::print_size(list);
            essentials::logger("saving data structure to disk...");
            essentials::save<unsorted_list_succinct_rmq>(list, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        unsorted_list_succinct_rmq list;
        essentials::logger("loading data structure from disk...");
        essentials::load(list, output_filename);
        essentials::logger("DONE");

        std::cout << "using " << list.bytes() << " bytes" << std::endl;

        std::vector<id_type> topk(max_k);
        auto queries = gen_random_queries(num_queries, doc_ids.size());
        std::cout << "testing top-" << k << " " << num_queries
                  << " random queries..." << std::endl;

        for (auto q : queries) {
            auto expected = naive_topk(doc_ids, q, k);
            uint32_t num_elements = list.topk(q, k, topk);

            if (expected.size() != num_elements) {
                std::cout << "Error: expected " << expected.size()
                          << " topk elements but got " << num_elements
                          << std::endl;
                return 1;
            }

            for (uint32_t i = 0; i != num_elements; ++i) {
                if (topk[i] != expected[i]) {
                    std::cout << "Error: expected " << expected[i]
                              << " but got " << topk[i] << std::endl;
                    return 1;
                }
            }
        }

        std::cout << "it's all good" << std::endl;
    }

    return 0;
}
