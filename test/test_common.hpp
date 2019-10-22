#pragma once

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../external/doctest/doctest/doctest.h"

#include <iostream>

#include "types.hpp"
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

}  // namespace testing
}  // namespace autocomplete