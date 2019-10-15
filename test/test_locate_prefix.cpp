#include <iostream>

#include "types.hpp"
#include "../benchmark/benchmark_common.hpp"

using namespace autocomplete;

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

template <typename Dictionary, typename Index>
int test_locate_prefix(Dictionary const& dict, Index const& index,
                       std::vector<std::string> const& queries,
                       std::vector<std::string> const& strings) {
    for (auto const& query : queries) {
        std::string query_copy = query;
        range expected = locate_prefix(strings, query);

        // std::cout << "query: '" << query << "'" << std::endl;
        completion_type prefix;
        byte_range suffix;
        parse(dict, query_copy, prefix, suffix);

        // print_completion(prefix);
        // std::cout << std::endl;
        // print(suffix);
        // std::cout << std::endl;

        range suffix_lex_range = dict.locate_prefix(suffix);
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range got = index.locate_prefix(prefix, suffix_lex_range);

        if ((got.begin != expected.begin) or (got.end != expected.end)) {
            std::cout << "Error for query '" << query << "': ";
            std::cout << "expected [" << expected.begin << "," << expected.end
                      << ") but got [" << got.begin << "," << got.end << ")"
                      << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    int mandatory = 2;
    if (argc < mandatory) {
        std::cout << argv[0] << " <collection_basename>" << std::endl;
        return 1;
    }

    parameters params;
    params.collection_basename = argv[1];
    params.load();

    fc_dictionary_type dict;
    {
        fc_dictionary_type::builder builder(params);
        builder.build(dict);
    }

    std::vector<std::string> strings;

    {
        essentials::logger("loading all strings...");
        std::string line;
        strings.reserve(params.num_completions);
        std::ifstream input((params.collection_basename).c_str(),
                            std::ios_base::in);
        for (uint32_t i = 0; i != params.num_completions; ++i) {
            if (!std::getline(input, line)) break;
            auto s = line.substr(line.find(' ') + 1, line.size());
            strings.push_back(s);
        }
        input.close();
        essentials::logger("loaded " + std::to_string(strings.size()) +
                           " strings");
    }

    uint32_t max_num_queries = std::atoi(argv[2]);
    std::vector<std::string> queries;
    essentials::logger("loading queries...");
    uint32_t num_queries =
        load_queries(queries, max_num_queries, true, std::cin);
    essentials::logger("loaded " + std::to_string(num_queries) + " queries");

    {
        // typedef uint64_completion_trie completion_trie_type;
        typedef ef_completion_trie completion_trie_type;

        completion_trie_type index;
        {
            completion_trie_type::builder builder(params);
            builder.build(index);
        }
        essentials::logger("testing locate_prefix() for completion_trie...");
        int ret = test_locate_prefix(dict, index, queries, strings);
        if (ret) return 1;
        essentials::logger("it's all good");
    }

    {
        integer_fc_dictionary_type index;
        {
            integer_fc_dictionary_type::builder builder(params);
            builder.build(index);
        }
        essentials::logger(
            "testing locate_prefix() for integer_fc_dictionary...");
        int ret = test_locate_prefix(dict, index, queries, strings);
        if (ret) return 1;
        essentials::logger("it's all good");
    }

    return 0;
}
