#pragma once

#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"
#include "scored_completion_set.hpp"
#include "constants.hpp"

namespace autocomplete {

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename InvertedIndex>
struct autocomplete2 {
    typedef scored_string_pool::iterator iterator_type;

    autocomplete2() {
        m_pool.resize(constants::POOL_SIZE, constants::MAX_K);
        m_topk_completion_set.resize(constants::MAX_K,
                                     constants::MAX_NUM_TERMS_PER_QUERY);
        m_pref_topk_scores.resize(constants::MAX_K);
        m_conj_topk_completions.resize(constants::MAX_K,
                                       constants::MAX_NUM_TERMS_PER_QUERY);
    }

    autocomplete2(parameters const& params)
        : autocomplete2() {
        typename Completions::builder cm_builder(params);
        typename Dictionary::builder di_builder(params);
        typename InvertedIndex::builder ii_builder(params);

        auto const& doc_ids = cm_builder.doc_ids();
        m_unsorted_docs_list.build(doc_ids);
        m_unsorted_minimal_docs_list.build(ii_builder.minimal_doc_ids());

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
        if (suffix_lex_range.is_invalid()) return m_pool.begin();

        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        if (r.is_invalid()) return m_pool.begin();

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
            extract_completions(num_completions);

        } else {
            if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions = conjunctive_topk(it, suffix_lex_range, k,
                                                   m_topk_completion_set);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions = conjunctive_topk(it, suffix_lex_range, k,
                                                   m_topk_completion_set);
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
                    suffix_lex_range, k, m_conj_topk_completions.scores(),
                    true  // must return unique results
                );
            } else {
                if (prefix.size() == 1) {  // we've got nothing to intersect
                    auto it = m_inverted_index.iterator(prefix.front() - 1);
                    num_conj_topk_completions = conjunctive_topk(
                        it, suffix_lex_range, k, m_conj_topk_completions);
                } else {
                    auto it = m_inverted_index.intersection_iterator(prefix);
                    num_conj_topk_completions = conjunctive_topk(
                        it, suffix_lex_range, k, m_conj_topk_completions);
                }
            }

            num_completions =
                merge_scores(num_pref_topk_completions,
                             num_conj_topk_completions, k, num_terms == 1);
        } else {
            num_completions = num_pref_topk_completions;
            m_pool.scores().swap(m_pref_topk_scores);
            extract_completions(num_completions);
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
            extract_completions(num_completions);

        } else {
            if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions = conjunctive_topk(it, suffix_lex_range, k,
                                                   m_topk_completion_set);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions = conjunctive_topk(it, suffix_lex_range, k,
                                                   m_topk_completion_set);
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
               essentials::vec_bytes(m_docid_to_lexid) +
               m_inverted_index.bytes();
    }

    void print_stats() const;

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_completions);
        visitor.visit(m_unsorted_docs_list);
        visitor.visit(m_unsorted_minimal_docs_list);
        visitor.visit(m_dictionary);
        visitor.visit(m_inverted_index);
        visitor.visit(m_docid_to_lexid);
    }

private:
    Completions m_completions;
    UnsortedDocsList m_unsorted_docs_list;
    UnsortedDocsList m_unsorted_minimal_docs_list;
    Dictionary m_dictionary;
    InvertedIndex m_inverted_index;
    std::vector<uint32_t> m_docid_to_lexid;
    scored_string_pool m_pool;
    scored_completion_set m_topk_completion_set;

    std::vector<id_type> m_pref_topk_scores;
    scored_completion_set m_conj_topk_completions;

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

    template <typename Iterator>
    uint32_t conjunctive_topk(Iterator& it, const range r, const uint32_t k,
                              scored_completion_set& completion_set) {
        auto& topk_scores = completion_set.scores();
        auto& completions = completion_set.completions();
        auto& sizes = completion_set.sizes();
        uint32_t i = 0;

        for (; it.has_next(); ++it) {
            id_type doc_id = *it;
            id_type lex_id = m_docid_to_lexid[doc_id];
            uint32_t size = m_completions.extract(lex_id, completions[i]);

            bool found = false;
            for (uint32_t j = 0; j != size and !found; ++j) {
                if (r.contains(completions[i][j])) found = true;
            }

            if (found) {
                topk_scores[i] = doc_id;
                sizes[i] = size;
                ++i;
                if (i == k) break;
            }
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
                          const uint32_t k, bool extract_all) {
        auto const& conj_topk_scores = m_conj_topk_completions.scores();
        auto& topk_scores = m_pool.scores();

        uint32_t n = set_union(m_pref_topk_scores, num_pref_topk_completions,
                               conj_topk_scores, num_conj_topk_completions,
                               topk_scores, k);

        auto& completions = m_topk_completion_set.completions();
        auto& sizes = m_topk_completion_set.sizes();
        auto& conjunctive_completions = m_conj_topk_completions.completions();
        auto& conjunctive_sizes = m_conj_topk_completions.sizes();

        uint32_t pos = 0;
        for (uint32_t i = 0; i != n; ++i) {
            id_type doc_id = topk_scores[i];
            bool found = false;

            for (; !extract_all and pos != num_conj_topk_completions; ++pos) {
                if (doc_id == conj_topk_scores[pos]) {
                    found = true;
                    break;
                }
                if (doc_id < conj_topk_scores[pos]) break;
            }

            if (found) {
                completions[i].swap(conjunctive_completions[pos]);
                sizes[i] = conjunctive_sizes[pos];
            } else {
                id_type lex_id = m_docid_to_lexid[doc_id];
                uint8_t size = m_completions.extract(lex_id, completions[i]);
                sizes[i] = size;
            }
        }

        return n;
    }
};
}  // namespace autocomplete