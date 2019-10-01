#pragma once

#include "util_types.hpp"
#include "scored_strings_pool.hpp"

namespace autocomplete {

template <typename CompletionTrie, typename UnsortedDocsList,
          typename Dictionary, typename InvertedIndex, typename ForwardIndex>
struct autocomplete {
    static const size_t POOL_SIZE = 1024;
    static const uint32_t MAX_K = 15;
    typedef scored_strings_pool::iterator iterator_type;

    autocomplete() {
        m_pool.resize(POOL_SIZE, MAX_K);
    }

    autocomplete(parameters const& params)
        : autocomplete() {
        typename CompletionTrie::builder ct_builder(params);
        typename Dictionary::builder di_builder(params);
        typename InvertedIndex::builder ii_builder(params);
        typename ForwardIndex::builder fi_builder(params);

        m_unsorted_docs_list.build(ct_builder.doc_ids());
        m_unsorted_minimal_docs_list.build(ii_builder.minimal_doc_ids());

        ct_builder.build(m_completion_trie);
        di_builder.build(m_dictionary);
        ii_builder.build(m_inverted_index);
        fi_builder.build(m_forward_index);
    }

    iterator_type prefix_topk(std::string& query, uint32_t k) {
        assert(k <= MAX_K);
        completion_type prefix;
        byte_range suffix;
        parse(query, prefix, suffix);
        auto& topk = m_pool.scores();
        range r = m_completion_trie.prefix_range(prefix);
        uint32_t num_completions = m_unsorted_docs_list.topk(r, k, topk);
        return extract_strings(num_completions, topk);
    }

    iterator_type conjunctive_topk(std::string& query, uint32_t k) {
        assert(k <= MAX_K);
        completion_type prefix;
        byte_range suffix;
        uint32_t num_terms = parse(query, prefix, suffix);
        assert(num_terms > 0);

        auto& topk = m_pool.scores();
        uint32_t num_completions = 0;

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);

        // std::cout << "num_terms " << num_terms << std::endl;
        // std::cout << "prefix is ";
        // for (auto x : prefix) {
        //     std::cout << x << " ";
        // }
        // std::cout << std::endl;
        // std::cout << "suffix is '" << std::string(suffix.begin, suffix.end)
        //           << "'" << std::endl;

        if (num_terms == 1) {  // special case

            num_completions = m_unsorted_minimal_docs_list.topk(
                suffix_lex_range, k, topk,
                true  // must return unique results
            );

        } else {
            prefix.pop_back();
            static const uint32_t max_size = m_inverted_index.num_docs();
            static std::vector<id_type> intersection(max_size);

            // NOTE: if n is large, then we may spend a lot of time here...
            // we may need an interator over the intersection
            uint64_t n = m_inverted_index.intersect(prefix, intersection);

            for (uint32_t i = 0; i != n; ++i) {
                id_type doc_id = intersection[i];
                auto it = m_forward_index.iterator(doc_id);
                if (it.contains(suffix_lex_range)) {
                    topk[num_completions++] = doc_id;
                    if (num_completions == k) break;
                }
            }
        }

        return extract_strings(num_completions, topk);
    }

    // for benchmarking
    iterator_type prefix_topk(std::string& query, uint32_t k,
                              std::vector<timer_type>& timers) {
        // step 0
        timers[0].start();
        assert(k <= MAX_K);
        completion_type prefix;
        byte_range suffix;
        parse(query, prefix, suffix);
        timers[0].stop();

        // step 1
        timers[1].start();
        range r = m_completion_trie.prefix_range(prefix);
        timers[1].stop();

        // step 2
        timers[2].start();
        auto& topk = m_pool.scores();
        uint32_t num_completions = m_unsorted_docs_list.topk(r, k, topk);
        timers[2].stop();

        // step 3
        timers[3].start();
        auto it = extract_strings(num_completions, topk);
        timers[3].stop();

        return it;
    }

    // for benchmarking
    iterator_type conjunctive_topk(std::string& query, uint32_t k,
                                   std::vector<timer_type>& timers) {
        // step 0
        timers[0].start();
        assert(k <= MAX_K);
        completion_type prefix;
        byte_range suffix{0, 0};
        uint32_t num_terms = parse(query, prefix, suffix);
        assert(num_terms > 0);
        timers[0].stop();

        auto& topk = m_pool.scores();
        uint32_t num_completions = 0;

        // step 1
        timers[1].start();
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        timers[1].stop();

        // step 2
        timers[2].start();
        if (num_terms == 1) {  // special case

            num_completions = m_unsorted_minimal_docs_list.topk(
                suffix_lex_range, k, topk,
                true  // must return unique results
            );

        } else {
            prefix.pop_back();
            static const uint32_t max_size = m_inverted_index.num_docs();
            static std::vector<id_type> intersection(max_size);
            uint64_t n = m_inverted_index.intersect(prefix, intersection);
            for (uint32_t i = 0; i != n; ++i) {
                id_type doc_id = intersection[i];
                auto it = m_forward_index.iterator(doc_id);
                if (it.contains(suffix_lex_range)) {
                    topk[num_completions++] = doc_id;
                    if (num_completions == k) break;
                }
            }
        }
        timers[2].stop();

        // step 3
        timers[3].start();
        auto it = extract_strings(num_completions, topk);
        timers[3].stop();

        return it;
    }

    size_t bytes() const {
        return m_completion_trie.bytes() + m_unsorted_docs_list.bytes() +
               m_unsorted_minimal_docs_list.bytes() + m_dictionary.bytes() +
               m_inverted_index.bytes() + m_forward_index.bytes();
    }

    void print_stats() const;

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_completion_trie);
        visitor.visit(m_unsorted_docs_list);
        visitor.visit(m_unsorted_minimal_docs_list);
        visitor.visit(m_dictionary);
        visitor.visit(m_inverted_index);
        visitor.visit(m_forward_index);
    }

private:
    CompletionTrie m_completion_trie;
    UnsortedDocsList m_unsorted_docs_list;
    UnsortedDocsList m_unsorted_minimal_docs_list;
    Dictionary m_dictionary;
    InvertedIndex m_inverted_index;
    ForwardIndex m_forward_index;
    scored_strings_pool m_pool;

    uint32_t parse(std::string& query, completion_type& prefix,
                   byte_range& suffix) {
        m_pool.clear();
        m_pool.init();
        uint32_t num_terms = parse_query(query);
        prefix.reserve(num_terms);
        forward_byte_range_iterator it(string_to_byte_range(query));
        for (uint32_t i = 0; i != num_terms; ++i) {
            suffix = it.next();
            id_type term_id = m_dictionary.locate(suffix);
            prefix.push_back(term_id);
        }
        return num_terms;
    }

    iterator_type extract_strings(uint32_t num_completions,
                                  std::vector<id_type> const& topk) {
        for (uint32_t i = 0; i != num_completions; ++i) {
            id_type doc_id = topk[i];
            auto it = m_forward_index.permuting_iterator(doc_id);
            uint64_t offset = m_pool.bytes();
            uint8_t* decoded = m_pool.data() + offset;
            for (uint32_t j = 0; j != it.size(); ++j, ++it) {
                id_type term_id = *it;
                uint8_t len = m_dictionary.extract(term_id, decoded);
                decoded += len;
                offset += len;
                if (j != it.size() - 1) {
                    *decoded++ = ' ';
                    offset++;
                }
            }
            m_pool.push_back_offset(offset);
        }
        assert(m_pool.size() == num_completions);
        return m_pool.begin();
    }
};
}  // namespace autocomplete