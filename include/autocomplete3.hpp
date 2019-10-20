#pragma once

#include "util_types.hpp"
#include "building_util.hpp"
#include "compact_vector.hpp"
#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"
#include "min_heap.hpp"
#include "constants.hpp"

namespace autocomplete {

/*
During the conjunctive step, maintain a min-heap of iterators,
one iterator for each termID in the lexicographic range of the
last token of the query.
*/

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename InvertedIndex>
struct autocomplete3 {
    typedef scored_string_pool::iterator iterator_type;
    typedef min_heap<typename InvertedIndex::iterator_type,
                     iterator_comparator<typename InvertedIndex::iterator_type>>
        min_priority_queue_type;

    autocomplete3() {
        m_pool.resize(constants::POOL_SIZE, constants::MAX_K);
        m_topk_completion_set.resize(constants::MAX_K,
                                     2 * constants::MAX_NUM_TERMS_PER_QUERY);
    }

    autocomplete3(parameters const& params)
        : autocomplete3() {
        typename Completions::builder cm_builder(params);
        typename Dictionary::builder di_builder(params);
        typename InvertedIndex::builder ii_builder(params);

        auto const& doc_ids = cm_builder.doc_ids();
        m_unsorted_docs_list.build(doc_ids);

        {
            essentials::logger("building map from doc_id to lex_id...");
            uint64_t n = doc_ids.size();
            typedef std::vector<std::pair<id_type, id_type>> id_map_type;
            id_map_type ids;
            ids.reserve(n);
            for (id_type lex_id = 0; lex_id != n; ++lex_id) {
                ids.emplace_back(lex_id, doc_ids[lex_id]);
            }
            std::sort(ids.begin(), ids.end(), [](auto const& l, auto const& r) {
                return l.second < r.second;
            });
            m_docid_to_lexid.build(
                util::first_iterator<typename id_map_type::const_iterator>(
                    ids.begin()),
                ids.size());
            essentials::logger("DONE");
        }

        cm_builder.build(m_completions);
        di_builder.build(m_dictionary);
        ii_builder.build(m_inverted_index);
    }

    iterator_type prefix_topk(std::string const& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        parse(m_dictionary, query, prefix, suffix);

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

    iterator_type conjunctive_topk(std::string const& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        parse(m_dictionary, query, prefix, suffix);

        uint32_t num_completions = 0;
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();

        if (prefix.size() == 1) {  // we've got nothing to intersect
            auto it = m_inverted_index.iterator(prefix.front() - 1);
            num_completions = conjunctive_topk(it, suffix_lex_range, k);
        } else {
            auto it = m_inverted_index.intersection_iterator(prefix);
            num_completions = conjunctive_topk(it, suffix_lex_range, k);
        }

        extract_completions(num_completions);
        return extract_strings(num_completions);
    }

    iterator_type topk(std::string const& query, const uint32_t k) {
        assert(k <= constants::MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        uint32_t num_terms = parse(m_dictionary, query, prefix, suffix);
        assert(num_terms > 0);

        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();

        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);

        uint32_t num_completions = 0;
        if (!r.is_invalid()) {
            num_completions = m_unsorted_docs_list.topk(r, k, m_pool.scores());
        }

        if (num_completions < k) {
            if (num_terms == 1) {  // we've got nothing to intersect
                iterator it(0, m_inverted_index.num_docs());
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            } else if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            }
        }

        extract_completions(num_completions);
        return extract_strings(num_completions);
    }

    iterator_type topk(std::string const& query, const uint32_t k,
                       std::vector<timer_type>& timers) {
        assert(k <= constants::MAX_K);

        timers[0].start();
        init();
        completion_type prefix;
        byte_range suffix;
        uint32_t num_terms = parse(m_dictionary, query, prefix, suffix);
        assert(num_terms > 0);
        timers[0].stop();

        timers[1].start();
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        uint32_t num_completions = 0;
        if (!r.is_invalid()) {
            num_completions = m_unsorted_docs_list.topk(r, k, m_pool.scores());
        }
        timers[1].stop();

        timers[2].start();
        if (num_completions < k) {
            if (num_terms == 1) {  // we've got nothing to intersect
                iterator it(0, m_inverted_index.num_docs());
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            } else if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            }
        }
        timers[2].stop();

        timers[3].start();
        extract_completions(num_completions);
        auto it = extract_strings(num_completions);
        timers[3].stop();

        return it;
    }

    // for benchmarking
    iterator_type prefix_topk(std::string const& query, uint32_t const k,
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
    iterator_type conjunctive_topk(std::string const& query, uint32_t const k,
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
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();
        timers[1].stop();

        // step 2
        timers[2].start();
        if (prefix.size() == 1) {  // we've got nothing to intersect
            auto it = m_inverted_index.iterator(prefix.front() - 1);
            num_completions = conjunctive_topk(it, suffix_lex_range, k);
        } else {
            auto it = m_inverted_index.intersection_iterator(prefix);
            num_completions = conjunctive_topk(it, suffix_lex_range, k);
        }
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
               m_dictionary.bytes() + m_docid_to_lexid.bytes() +
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
    InvertedIndex m_inverted_index;
    compact_vector m_docid_to_lexid;

    scored_string_pool m_pool;
    completion_set m_topk_completion_set;

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
            auto doc_id = topk_scores[i];
            auto lex_id = m_docid_to_lexid[doc_id];
            uint8_t size = m_completions.extract(lex_id, completions[i]);
            sizes[i] = size;
        }
    }

    template <typename Iterator>
    uint32_t conjunctive_topk(Iterator& it, const range r, const uint32_t k) {
        assert(!r.is_invalid());

        auto& topk_scores = m_pool.scores();
        min_priority_queue_type q;
        q.reserve(r.end - r.begin + 1);  // inclusive range
        assert(r.begin > 0);
        for (uint64_t term_id = r.begin; term_id <= r.end; ++term_id) {
            q.push_back(m_inverted_index.iterator(term_id - 1));
        }
        q.make_heap();

        uint32_t results = 0;
        for (; it.has_next() and !q.empty(); ++it) {
            auto doc_id = *it;

            bool found = false;
            while (!q.empty() and !found) {
                auto& z = q.top();
                auto val = *z;
                if (val > doc_id) break;
                if (val < doc_id) {
                    val = z.next_geq(doc_id);
                    if (!z.has_next()) {
                        q.pop();
                    } else {
                        q.heapify();
                    }
                }
                if (val == doc_id) found = true;
            }

            if (found) {
                topk_scores[results++] = doc_id;
                if (results == k) break;
            }
        }

        return results;
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
                auto term_id = c[j];
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
};
}  // namespace autocomplete