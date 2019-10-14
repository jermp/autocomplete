#include <iostream>

#include "types.hpp"

using namespace autocomplete;

id_type locate(std::vector<std::string> const& terms, std::string const& t) {
    return std::distance(terms.begin(),
                         std::lower_bound(terms.begin(), terms.end(), t)) +
           1;
}

range locate_prefix(std::vector<std::string> const& terms,
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
        terms.begin(), std::lower_bound(terms.begin(), terms.end(), p, comp_l));
    r.end =
        std::distance(terms.begin(),
                      std::upper_bound(terms.begin(), terms.end(), p, comp_r)) -
        1;

    return r;
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

    {
        // build, print and write
        fc_dictionary_B16::builder builder(params);
        fc_dictionary_B16 dict;
        builder.build(dict);
        std::cout << "using " << dict.bytes() << " bytes" << std::endl;

        if (output_filename) {
            // essentials::print_size(dict);
            essentials::logger("saving data structure to disk...");
            essentials::save<fc_dictionary_B16>(dict, output_filename);
            essentials::logger("DONE");
        }
    }

    {
        if (output_filename) {
            fc_dictionary_B16 dict;
            essentials::logger("loading data structure from disk...");
            essentials::load(dict, output_filename);
            essentials::logger("DONE");
            // essentials::print_size(dict);
            std::cout << "using " << dict.bytes() << " bytes" << std::endl;

            // test locate() and extract for all strings
            std::vector<std::string> terms;
            terms.reserve(params.num_terms);
            std::ifstream input((params.collection_basename + ".dict").c_str(),
                                std::ios_base::in);
            if (!input.good()) {
                throw std::runtime_error("File not found");
            }
            std::string term;
            term.reserve(256 + 1);
            input >> term;
            while (input) {
                terms.push_back(std::move(term));
                input >> term;
            }
            input.close();

            std::cout << "terms.size() " << terms.size() << std::endl;

            std::vector<uint8_t> decoded(256 + 1);  // assume this is enough

            for (auto const& t : terms) {
                id_type expected = locate(terms, t);
                id_type got = dict.locate(string_to_byte_range(t));

                std::cout << "locating term '" << t << "'" << std::endl;
                if (got != expected) {
                    std::cout << "Error: expected id " << expected << ","
                              << " but got id " << got << std::endl;
                    return 1;
                }

                std::cout << "extracting term '" << t << "'" << std::endl;
                uint8_t string_len = dict.extract(got, decoded.data());

                if (string_len != t.size()) {
                    std::cout << "Error: expected size " << t.size() << ","
                              << " but got size " << string_len << std::endl;
                    return 1;
                }

                auto s = reinterpret_cast<char const*>(decoded.data());
                for (uint8_t i = 0; i != string_len; ++i) {
                    if (t[i] != s[i]) {
                        std::cout << "Error: expected char " << t[i]
                                  << " but got " << s[i] << std::endl;
                        return 1;
                    }
                }

                std::cout << "lexicographic id of '" << t << "' is " << got
                          << std::endl;
            }

            // test locate_prefix() for all strings
            std::string prefix;
            prefix.reserve(256 + 1);
            for (auto const& t : terms) {
                uint32_t n = t.size();
                for (uint32_t prefix_len = 1; prefix_len <= n; ++prefix_len) {
                    prefix.clear();
                    for (uint32_t i = 0; i != prefix_len; ++i) {
                        prefix.push_back(t[i]);
                    }

                    std::cout << "locating prefix '" << prefix << "'"
                              << std::endl;
                    range expected = locate_prefix(terms, prefix);
                    range got =
                        dict.locate_prefix(string_to_byte_range(prefix));

                    if ((got.begin != expected.begin) or
                        (got.end != expected.end)) {
                        std::cout << "Error for prefix '" << prefix
                                  << "' : expected [" << expected.begin << ","
                                  << expected.end << "] but got [" << got.begin
                                  << "," << got.end << "]" << std::endl;
                        return 1;
                    }

                    std::cout << "prefix range of '" << prefix << "' is ["
                              << got.begin << "," << got.end << "]"
                              << std::endl;
                }
            }
        }
    }

    return 0;
}
