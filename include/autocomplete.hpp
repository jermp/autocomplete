#pragma once

#include "util_types.hpp"
#include "scored_string_pool.hpp"

namespace autocomplete {

template <typename CompletionTrie, typename UnsortedDocsList,
          typename Dictionary, typename InvertedIndex, typename ForwardIndex>
struct autocomplete {
    static const size_t POOL_SIZE = 1024;
    static const uint32_t MAX_K = 15;
    typedef scored_string_pool::iterator iterator_type;

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

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);

        // NOTE: because the completion_trie works with 1-based ids
        // (id 0 is reserved for null terminator)
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;

        range r = m_completion_trie.locate_prefix(prefix, suffix_lex_range);
        auto& topk = m_pool.scores();
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

        if (num_terms == 1) {  // special case

            suffix_lex_range.end += 1;
            num_completions = m_unsorted_minimal_docs_list.topk(
                suffix_lex_range, k, topk,
                true  // must return unique results
            );

        } else {
            if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, topk, k);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, topk, k);
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
        byte_range suffix{0, 0};
        parse(query, prefix, suffix);
        timers[0].stop();

        // step 1
        timers[1].start();
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completion_trie.locate_prefix(prefix, suffix_lex_range);
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

            suffix_lex_range.end += 1;
            num_completions = m_unsorted_minimal_docs_list.topk(
                suffix_lex_range, k, topk,
                true  // must return unique results
            );

        } else {
            if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, topk, k);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, topk, k);
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
    scored_string_pool m_pool;

    uint32_t parse(std::string& query, completion_type& prefix,
                   byte_range& suffix) {
        m_pool.clear();
        m_pool.init();
        uint32_t num_terms = parse_query(query);
        assert(num_terms > 0);
        prefix.reserve(num_terms);
        forward_byte_range_iterator it(string_to_byte_range(query));
        for (uint32_t i = 0; i != num_terms; ++i) {
            suffix = it.next();
            if (i == num_terms - 1) break;
            id_type term_id = m_dictionary.locate(suffix);
            prefix.push_back(term_id);
        }
        return num_terms;
    }

    template <typename Iterator>
    uint32_t conjunctive_topk(Iterator& it, range r, std::vector<id_type>& topk,
                              uint32_t k) {
        uint32_t i = 0;
        while (it.has_next()) {
            id_type doc_id = *it;
            if (m_forward_index.contains(doc_id, r)) {
                topk[i++] = doc_id;
                if (i == k) break;
            }
            ++it;
        }
        return i;
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