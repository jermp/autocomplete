#pragma once

#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"
#include "scored_completion_set.hpp"
#include "min_heap.hpp"
#include "constants.hpp"

namespace autocomplete {

/* Bast and Weber approach. */

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename BlockedInvertedIndex>
struct autocomplete4 {
    typedef scored_string_pool::iterator iterator_type;

    autocomplete4() {
        m_pool.resize(constants::POOL_SIZE, constants::MAX_K);
        m_topk_completion_set.resize(constants::MAX_K,
                                     constants::MAX_NUM_TERMS_PER_QUERY);
        m_pref_topk_scores.resize(constants::MAX_K);
        m_conj_topk_scores.resize(constants::MAX_K);
    }

    autocomplete4(parameters const& params, float c)
        : autocomplete4() {
        typename Completions::builder cm_builder(params);
        typename Dictionary::builder di_builder(params);
        typename BlockedInvertedIndex::builder ii_builder(params, c);

        auto const& doc_ids = cm_builder.doc_ids();
        m_unsorted_docs_list.build(doc_ids);

        {
            uint64_t n = doc_ids.size();
            std::vector<std::pair<id_type, id_type>> ids;
            ids.reserve(n);
            m_docid_to_lexid.reserve(n);
            for (id_type lex_id = 0; lex_id != n; ++lex_id) {
                ids.emplace_back(lex_id, doc_ids[lex_id]);
            }
            std::sort(ids.begin(), ids.end(), [](auto const& l, auto const& r) {
                return l.second < r.second;
            });
            for (id_type doc_id = 0; doc_id != n; ++doc_id) {
                m_docid_to_lexid.push_back(ids[doc_id].first);
            }
        }

        cm_builder.build(m_completions);
        di_builder.build(m_dictionary);
        ii_builder.build(m_inverted_index);
    }

    iterator_type prefix_topk(std::string& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        parse(m_dictionary, query, prefix, suffix);

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (is_invalid(suffix_lex_range)) return m_pool.begin();

        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        if (is_invalid(r)) return m_pool.begin();

        uint32_t num_completions =
            m_unsorted_docs_list.topk(r, k, m_pool.scores());
        extract_completions(num_completions);
        return extract_strings(num_completions);
    }

    iterator_type conjunctive_topk(std::string& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        parse(m_dictionary, query, prefix, suffix);

        uint32_t num_completions = 0;
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (is_invalid(suffix_lex_range)) return m_pool.begin();

        num_completions =
            conjunctive_topk(prefix, suffix_lex_range, k, m_pool.scores());

        extract_completions(num_completions);
        return extract_strings(num_completions);
    }

    iterator_type topk(std::string& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        parse(m_dictionary, query, prefix, suffix);

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (is_invalid(suffix_lex_range)) return m_pool.begin();

        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);

        uint32_t num_pref_topk_completions = 0;
        if (!is_invalid(r)) {
            num_pref_topk_completions =
                m_unsorted_docs_list.topk(r, k, m_pref_topk_scores);
        }

        uint32_t num_completions = 0;

        if (num_pref_topk_completions < k) {
            uint32_t num_conj_topk_completions = conjunctive_topk(
                prefix, suffix_lex_range, k, m_conj_topk_scores);
            num_completions = merge_scores(num_pref_topk_completions,
                                           num_conj_topk_completions, k);
        } else {
            num_completions = num_pref_topk_completions;
            m_pool.scores().swap(m_pref_topk_scores);
        }

        extract_completions(num_completions);
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
        if (is_invalid(suffix_lex_range)) return m_pool.begin();

        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        if (is_invalid(r)) return m_pool.begin();
        timers[1].stop();

        // step 2
        timers[2].start();
        uint32_t num_completions =
            m_unsorted_docs_list.topk(r, k, m_pool.scores());
        timers[2].stop();

        // step 3
        timers[3].start();
        extract_completions(num_completions);
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
        parse(m_dictionary, query, prefix, suffix);
        timers[0].stop();

        uint32_t num_completions = 0;

        // step 1
        timers[1].start();
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (is_invalid(suffix_lex_range)) return m_pool.begin();
        timers[1].stop();

        // step 2
        timers[2].start();
        num_completions =
            conjunctive_topk(prefix, suffix_lex_range, k, m_pool.scores());
        timers[2].stop();

        // step 3
        timers[3].start();
        extract_completions(num_completions);
        auto it = extract_strings(num_completions);
        timers[3].stop();

        return it;
    }

    size_t bytes() const {
        return m_completions.bytes() + m_unsorted_docs_list.bytes() +
               m_dictionary.bytes() + essentials::vec_bytes(m_docid_to_lexid) +
               m_inverted_index.bytes();
    }

    void print_stats() const;

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_completions);
        visitor.visit(m_unsorted_docs_list);
        visitor.visit(m_dictionary);
        visitor.visit(m_inverted_index);
        visitor.visit(m_docid_to_lexid);
    }

private:
    Completions m_completions;
    UnsortedDocsList m_unsorted_docs_list;
    Dictionary m_dictionary;
    BlockedInvertedIndex m_inverted_index;
    std::vector<uint32_t> m_docid_to_lexid;
    scored_string_pool m_pool;

    scored_completion_set m_topk_completion_set;
    std::vector<id_type> m_pref_topk_scores;
    std::vector<id_type> m_conj_topk_scores;

    void init() {
        m_pool.clear();
        m_pool.init();
        assert(m_pool.size() == 0);
    }

    // NOTE: this can be done more efficienctly exploiting
    // the fact that the strings to be extracted share a common
    // prefix, thus this task should be delegated to the
    // integer_fc_dictionary... (enchance the locality of the operation)
    // NOTE: this only work when used during the prefix_topk step.
    void extract_completions(const uint32_t num_completions) {
        auto const& topk_scores = m_pool.scores();
        auto& completions = m_topk_completion_set.completions();
        auto& sizes = m_topk_completion_set.sizes();
        for (uint32_t i = 0; i != num_completions; ++i) {
            id_type doc_id = topk_scores[i];
            id_type lex_id = m_docid_to_lexid[doc_id];
            uint8_t size = m_completions.extract(lex_id, completions[i]);
            sizes[i] = size;
        }
    }

    uint32_t conjunctive_topk(completion_type& prefix, const range suffix,
                              const uint32_t k,
                              std::vector<id_type>& topk_scores) {
        auto it = m_inverted_index.intersection_iterator(prefix, suffix);
        uint32_t i = 0;
        while (it.has_next()) {
            id_type doc_id = *it;
            if (it.intersects()) {
                topk_scores[i++] = doc_id;
                if (i == k) break;
            }
            ++it;
        }
        return i;
    }

    iterator_type extract_strings(const uint32_t num_completions) {
        auto const& completions = m_topk_completion_set.completions();
        auto const& sizes = m_topk_completion_set.sizes();
        for (uint32_t i = 0; i != num_completions; ++i) {
            auto const& c = completions[i];
            uint32_t size = sizes[i];
            uint64_t offset = m_pool.bytes();
            uint8_t* decoded = m_pool.data() + offset;
            for (uint32_t j = 0; j != size; ++j) {
                id_type term_id = c[j];
                uint8_t len = m_dictionary.extract(term_id, decoded);
                decoded += len;
                offset += len;
                if (j != size - 1) {
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