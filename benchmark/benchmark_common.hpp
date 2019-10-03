#pragma once

namespace autocomplete {

static const uint32_t runs = 10;

size_t load_queries(std::vector<std::string>& queries, uint32_t max_num_queries,
                    bool truncate_last = false, std::istream& is = std::cin) {
    std::string line, query;
    queries.reserve(max_num_queries);
    for (uint32_t i = 0; i != max_num_queries; ++i) {
        if (!std::getline(is, line)) break;
        auto query = line.substr(line.find(' ') + 1, line.size());

        if (truncate_last) {  // retain only first character
            char last = 0;
            while (query.size() and query.back() != ' ') {
                last = query.back();
                query.pop_back();
            }
            query.push_back(last);
        }

        queries.push_back(query);
    }
    return queries.size();
}

}  // namespace autocomplete