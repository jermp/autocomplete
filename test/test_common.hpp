#pragma once

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../external/doctest/doctest/doctest.h"

#include <iostream>

#include "types.hpp"
#include "probe.hpp"
#include "../benchmark/benchmark_common.hpp"

namespace autocomplete {
namespace testing {

static std::string test_filename(
    "../test_data/trec_05_efficiency_queries/"
    "trec_05_efficiency_queries.completions");

static std::string tmp_filename("tmp.bin");

id_type locate(std::vector<std::string> const& terms, std::string const& t) {
    return std::distance(terms.begin(),
                         std::lower_bound(terms.begin(), terms.end(), t)) +
           1;
}

range locate_prefix(std::vector<std::string> const& strings,
                    std::string const& p) {
    auto comp_l = [](std::string const& l, std::string const& r) {
        if (l.size() < r.size()) {
            return strncmp(l.c_str(), r.c_str(), l.size()) <= 0;
        }
        return strcmp(l.c_str(), r.c_str()) < 0;
    };

    auto comp_r = [](std::string const& l, std::string const& r) {
        if (l.size() < r.size()) {
            return strncmp(l.c_str(), r.c_str(), l.size()) < 0;
        }
        return strcmp(l.c_str(), r.c_str()) < 0;
    };

    range r;
    r.begin = std::distance(
        strings.begin(),
        std::lower_bound(strings.begin(), strings.end(), p, comp_l));
    r.end = std::distance(
        strings.begin(),
        std::upper_bound(strings.begin(), strings.end(), p, comp_r));

    return r;
}

typedef std::vector<id_type> term_ids;

std::vector<term_ids> gen_random_queries(uint32_t num_queries,
                                         uint32_t max_num_terms,
                                         uint32_t max_range_len) {
    assert(max_num_terms > 1);
    std::vector<term_ids> queries;
    queries.reserve(num_queries);
    essentials::uniform_int_rng<uint32_t> random_num_terms(2, max_num_terms);
    essentials::uniform_int_rng<uint32_t> random_term_id(1, max_range_len);

    for (uint32_t i = 0; i != num_queries; ++i) {
        term_ids q;
        uint32_t num_terms = random_num_terms.gen();
        q.reserve(num_terms);
        uint32_t num_distinct_terms = 0;
        while (true) {
            q.clear();
            for (uint32_t i = 0; i != num_terms; ++i) {
                q.push_back(random_term_id.gen());
            }
            std::sort(q.begin(), q.end());
            auto end = std::unique(q.begin(), q.end());
            num_distinct_terms = std::distance(q.begin(), end);
            if (num_distinct_terms >= 2) break;
        }
        q.resize(num_distinct_terms);
        queries.push_back(q);
    }

    return queries;
}

}  // namespace testing
}  // namespace autocomplete