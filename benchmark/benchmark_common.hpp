#pragma once

namespace autocomplete {

static const uint32_t runs = 5;

size_t load_queries(std::vector<std::string>& queries, uint32_t max_num_queries,
                    float percentage, std::istream& is = std::cin) {
    assert(percentage >= 0.0 and percentage <= 1.0);
    std::string line;
    queries.reserve(max_num_queries);
    for (uint32_t i = 0; i != max_num_queries; ++i) {
        if (!std::getline(is, line)) break;
        auto query = line.substr(line.find(' ') + 1, line.size());
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

}  // namespace autocomplete