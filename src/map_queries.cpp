#include <iostream>

#include "types.hpp"

using namespace autocomplete;

template <typename Dictionary>
completion_type parse(Dictionary const& dict, std::string const& query) {
    completion_type completion;
    byte_range_iterator it(string_to_byte_range(query));
    while (it.has_next()) {
        byte_range term = it.next();
        auto term_id = dict.locate(term);
        assert(term_id > 0);
        assert(term_id != global::invalid_term_id);
        completion.push_back(term_id - 1);
    }
    return completion;
}

int main(int argc, char** argv) {
    int mandatory = 2 + 1;
    if (argc < mandatory) {
        std::cout << argv[0] << " <collection_basename> <num_queries> < queries"
                  << std::endl;
        return 1;
    }

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    uint32_t num_queries = std::atoi(argv[2]);

    fc_dictionary_type dict;
    {
        fc_dictionary_type::builder builder(params);
        builder.build(dict);
    }

    std::string query;
    for (uint32_t i = 0; i != num_queries; ++i) {
        if (!std::getline(std::cin, query)) break;
        auto completion = parse(dict, query);
        std::cerr << completion.front();
        for (size_t i = 1; i != completion.size(); ++i) {
            std::cerr << "\t" << completion[i];
        }
        std::cerr << "\n";
    }

    return 0;
}