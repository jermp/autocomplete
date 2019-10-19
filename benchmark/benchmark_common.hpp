#pragma once

namespace autocomplete {

static const uint32_t runs = 10;

size_t load_queries(std::vector<std::string>& queries, uint32_t max_num_queries,
                    float percentage, std::istream& is = std::cin) {
    assert(percentage >= 0.0 and percentage <= 1.0);
    std::string line;
    queries.reserve(max_num_queries);
    for (uint32_t i = 0; i != max_num_queries; ++i) {
        if (!std::getline(is, line)) break;

        auto query = line.substr(line.find(' ') + 1, line.size());
        int32_t size = query.size() - 1;
        while (size >= 0 and query[size] != ' ') --size;
        auto last_token = query.substr(size + 1, query.size() - size);
        uint32_t num_chars =
            last_token.size() - std::ceil(last_token.size() * percentage);
        char first = last_token.front();
        for (uint32_t i = 0; i != num_chars; ++i) last_token.pop_back();

        // retain at least one char
        if (last_token.empty()) last_token.push_back(first);
        assert(last_token.size() > 0);

        queries.push_back(query.substr(0, size + 1) + last_token);
    }
    return queries.size();
}

}  // namespace autocomplete