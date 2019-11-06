#pragma once

#include "../external/cmd_line_parser/include/parser.hpp"

namespace autocomplete {

static const uint32_t runs = 5;

size_t load_queries(std::vector<std::string>& queries, uint32_t max_num_queries,
                    float percentage, std::istream& is = std::cin) {
    assert(percentage >= 0.0 and percentage <= 1.0);
    std::string query;
    queries.reserve(max_num_queries);
    for (uint32_t i = 0; i != max_num_queries; ++i) {
        if (!std::getline(is, query)) break;
        assert(query.size() > 0);
        size_t size = query.size() - 1;
        while (size > 0 and query[size] != ' ') --size;
        size_t last_token_size = query.size() - size;
        size_t end = size + std::ceil(last_token_size * percentage) + 1 +
                     1;  // retain at least one char
        for (size = query.size(); size > end; --size) query.pop_back();
        queries.push_back(query);
    }
    return queries.size();
}

void configure_parser_for_benchmarking(cmd_line_parser::parser& parser) {
    parser.add("type", "Index type.");
    parser.add("k", "top-k value.");
    parser.add("index_filename", "Index filename.");
    parser.add("num_terms_per_query", "Number of terms per query.");
    parser.add("max_num_queries", "Maximum number of queries to execute.");
    parser.add("percentage",
               "A float in [0,1] specifying how much we keep of the last token "
               "in a query: n x 100 <=> n%, for n in [0,1].");
    parser.add("breakdown", "Collect timings breakdown.", "--breakdown");
}

}  // namespace autocomplete