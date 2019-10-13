#pragma once

#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"
#include "constants.hpp"

namespace autocomplete {

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename InvertedIndex, typename ForwardIndex>
struct autocomplete {
    typedef scored_string_pool::iterator iterator_type;

    autocomplete() {
        m_pool.resize(constants::POOL_SIZE, constants::MAX_K);
        m_pref_topk_scores.resize(constants::MAX_K);
        m_conj_topk_scores.resize(constants::MAX_K);
    }

    autocomplete(parameters const& params)
        : autocomplete() {
        typename Completions::builder cm_builder(params);
        typename Dictionary::builder di_builder(params);
        typename InvertedIndex::builder ii_builder(params);
        typename ForwardIndex::builder fi_builder(params);

        m_unsorted_docs_list.build(cm_builder.doc_ids());
        m_unsorted_minimal_docs_list.build(ii_builder.minimal_doc_ids());

        cm_builder.build(m_completions);
        di_builder.build(m_dictionary);
        ii_builder.build(m_inverted_index);
        fi_builder.build(m_forward_index);
    }

    iterator_type prefix_topk(std::string& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        parse(m_dictionary, query, prefix, suffix);

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();

        // NOTE: because the completion_trie works with 1-based ids
        // (id 0 is reserved for null terminator)
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        if (r.is_invalid()) return m_pool.begin();

        uint32_t num_completions =
            m_unsorted_docs_list.topk(r, k, m_pool.scores());
        return extract_strings(num_completions);
    }

    iterator_type conjunctive_topk(std::string& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        uint32_t num_terms = parse(m_dictionary, query, prefix, suffix);
        assert(num_terms > 0);

        uint32_t num_completions = 0;
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();

        if (num_terms == 1) {  // special case
            suffix_lex_range.end += 1;
            num_completions = m_unsorted_minimal_docs_list.topk(
                suffix_lex_range, k, m_pool.scores(),
                true  // must return unique results
            );
        } else {
            if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, k, m_pool.scores());
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, k, m_pool.scores());
            }
        }

        return extract_strings(num_completions);
    }

    iterator_type topk(std::string& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        uint32_t num_terms = parse(m_dictionary, query, prefix, suffix);
        assert(num_terms > 0);

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();

        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);

        uint32_t num_pref_topk_completions = 0;
        if (!r.is_invalid()) {
            num_pref_topk_completions =
                m_unsorted_docs_list.topk(r, k, m_pref_topk_scores);
        }

        uint32_t num_completions = 0;

        if (num_pref_topk_completions < k) {
            uint32_t num_conj_topk_completions = 0;

            if (num_terms == 1) {  // special case
                suffix_lex_range.begin -= 1;
                num_conj_topk_completions = m_unsorted_minimal_docs_list.topk(
                    suffix_lex_range, k, m_conj_topk_scores,
                    true  // must return unique results
                );
            } else {
                if (prefix.size() == 1) {  // we've got nothing to intersect
                    auto it = m_inverted_index.iterator(prefix.front() - 1);
                    num_conj_topk_completions = conjunctive_topk(
                        it, suffix_lex_range, k, m_conj_topk_scores);
                } else {
                    auto it = m_inverted_index.intersection_iterator(prefix);
                    num_conj_topk_completions = conjunctive_topk(
                        it, suffix_lex_range, k, m_conj_topk_scores);
                }
            }

            num_completions = merge_scores(num_pref_topk_completions,
                                           num_conj_topk_completions, k);

        } else {
            num_completions = num_pref_topk_completions;
            m_pool.scores().swap(m_pref_topk_scores);
        }

        return extract_strings(num_completions);
    }

    // for benchmarking
    iterator_type prefix_topk(std::string& query, uint32_t const k,
                              std::vector<timer_type>& timers) {
        // step 0
        timers[0].start();
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix{0, 0};
        parse(m_dictionary, query, prefix, suffix);
        timers[0].stop();

        // step 1
        timers[1].start();
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        if (r.is_invalid()) return m_pool.begin();
        timers[1].stop();

        // step 2
        timers[2].start();
        uint32_t num_completions =
            m_unsorted_docs_list.topk(r, k, m_pool.scores());
        timers[2].stop();

        // step 3
        timers[3].start();
        auto it = extract_strings(num_completions);
        timers[3].stop();

        return it;
    }

    // for benchmarking
    iterator_type conjunctive_topk(std::string& query, uint32_t const k,
                                   std::vector<timer_type>& timers) {
        // step 0
        timers[0].start();
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix{0, 0};
        uint32_t num_terms = parse(m_dictionary, query, prefix, suffix);
        assert(num_terms > 0);
        timers[0].stop();

        uint32_t num_completions = 0;

        // step 1
        timers[1].start();
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();

        timers[1].stop();

        // step 2
        timers[2].start();
        if (num_terms == 1) {  // special case

            suffix_lex_range.end += 1;
            num_completions = m_unsorted_minimal_docs_list.topk(
                suffix_lex_range, k, m_pool.scores(),
                true  // must return unique results
            );

        } else {
            if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, k, m_pool.scores());
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions =
                    conjunctive_topk(it, suffix_lex_range, k, m_pool.scores());
            }
        }
        timers[2].stop();

        // step 3
        timers[3].start();
        auto it = extract_strings(num_completions);
        timers[3].stop();

        return it;
    }

    size_t bytes() const {
        return m_completions.bytes() + m_unsorted_docs_list.bytes() +
               m_unsorted_minimal_docs_list.bytes() + m_dictionary.bytes() +
               m_inverted_index.bytes() + m_forward_index.bytes();
    }

    void print_stats() const;

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_completions);
        visitor.visit(m_unsorted_docs_list);
        visitor.visit(m_unsorted_minimal_docs_list);
        visitor.visit(m_dictionary);
        visitor.visit(m_inverted_index);
        visitor.visit(m_forward_index);
    }

private:
    Completions m_completions;
    UnsortedDocsList m_unsorted_docs_list;
    UnsortedDocsList m_unsorted_minimal_docs_list;
    Dictionary m_dictionary;
    InvertedIndex m_inverted_index;
    ForwardIndex m_forward_index;
    scored_string_pool m_pool;

    std::vector<id_type> m_pref_topk_scores;
    std::vector<id_type> m_conj_topk_scores;

    void init() {
        m_pool.clear();
        m_pool.init();
        assert(m_pool.size() == 0);
    }

    template <typename Iterator>
    uint32_t conjunctive_topk(Iterator& it, const range r, uint32_t const k,
                              std::vector<id_type>& topk_scores) {
        uint32_t results = 0;
        for (; it.has_next(); ++it) {
            id_type doc_id = *it;
            if (m_forward_index.intersects(doc_id, r)) {
                topk_scores[results++] = doc_id;
                if (results == k) break;
            }
        }
        return results;
    }

    iterator_type extract_strings(const uint32_t num_completions) {
        auto const& topk_scores = m_pool.scores();
        for (uint32_t i = 0; i != num_completions; ++i) {
            id_type doc_id = topk_scores[i];
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

    uint32_t merge_scores(const uint32_t num_pref_topk_completions,
                          const uint32_t num_conj_topk_completions,
                          const uint32_t k) {
        return set_union(m_pref_topk_scores, num_pref_topk_completions,
                         m_conj_topk_scores, num_conj_topk_completions,
                         m_pool.scores(), k);
    }
};
}  // namespace autocomplete