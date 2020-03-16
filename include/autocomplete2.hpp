#pragma once

#include "util_types.hpp"
#include "building_util.hpp"
#include "compact_vector.hpp"
#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"
#include "constants.hpp"

namespace autocomplete {

template <typename Completions, typename Dictionary, typename InvertedIndex>
struct autocomplete2 {
    typedef scored_string_pool::iterator iterator_type;

    autocomplete2() {
        m_pool.resize(constants::POOL_SIZE, constants::MAX_K);
        m_topk_completion_set.resize(constants::MAX_K,
                                     2 * constants::MAX_NUM_TERMS_PER_QUERY);
    }

    autocomplete2(parameters const& params)
        : autocomplete2() {
        typename Completions::builder cm_builder(params);
        typename Dictionary::builder di_builder(params);
        typename InvertedIndex::builder ii_builder(params);
        auto const& docid_to_lexid = cm_builder.docid_to_lexid();
        m_docid_to_lexid.build(docid_to_lexid.begin(), docid_to_lexid.size(),
                               util::ceil_log2(params.num_completions + 1));
        m_unsorted_docs_list.build(
            util::invert(docid_to_lexid, params.num_completions));
        m_unsorted_minimal_docs_list.build(ii_builder.minimal_doc_ids());
        cm_builder.build(m_completions);
        di_builder.build(m_dictionary);
        ii_builder.build(m_inverted_index);
    }

    template <typename Probe>
    iterator_type prefix_topk(std::string const& query, const uint32_t k,
                              Probe& probe) {
        assert(k <= constants::MAX_K);

        probe.start(0);
        init();
        completion_type prefix;
        byte_range suffix;
        constexpr bool must_find_prefix = true;
        if (!parse(m_dictionary, query, prefix, suffix, must_find_prefix)) {
            return m_pool.begin();
        }
        probe.stop(0);

        probe.start(1);
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        if (r.is_invalid()) return m_pool.begin();
        uint32_t num_completions =
            m_unsorted_docs_list.topk(r, k, m_pool.scores());
        probe.stop(1);

        probe.start(2);
        extract_completions(num_completions);
        auto it = extract_strings(num_completions);
        probe.stop(2);

        return it;
    }

    template <typename Probe>
    iterator_type conjunctive_topk(std::string const& query, const uint32_t k,
                                   Probe& probe) {
        assert(k <= constants::MAX_K);

        probe.start(0);
        init();
        completion_type prefix;
        byte_range suffix;
        constexpr bool must_find_prefix = false;
        parse(m_dictionary, query, prefix, suffix, must_find_prefix);
        probe.stop(0);

        probe.start(1);
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        if (suffix_lex_range.is_invalid()) return m_pool.begin();
        uint32_t num_completions = 0;
        if (prefix.size() == 0) {
            suffix_lex_range.end += 1;
            num_completions = m_unsorted_minimal_docs_list.topk(
                m_inverted_index, suffix_lex_range, k, m_pool.scores());
            extract_completions(num_completions);
        } else {
            suffix_lex_range.begin += 1;
            suffix_lex_range.end += 1;
            num_completions = conjunctive_topk(prefix, suffix_lex_range, k);
        }
        probe.stop(1);

        probe.start(2);
        auto it = extract_strings(num_completions);
        probe.stop(2);

        return it;
    }

    // iterator_type topk(std::string const& query, const uint32_t k) {
    //     assert(k <= constants::MAX_K);
    //     init();
    //     completion_type prefix;
    //     byte_range suffix;
    //     uint32_t num_terms = parse(m_dictionary, query, prefix, suffix);
    //     assert(num_terms > 0);

    //     range suffix_lex_range = m_dictionary.locate_prefix(suffix);
    //     if (suffix_lex_range.is_invalid()) return m_pool.begin();

    //     suffix_lex_range.begin += 1;
    //     suffix_lex_range.end += 1;
    //     range r = m_completions.locate_prefix(prefix, suffix_lex_range);

    //     uint32_t num_completions = 0;
    //     if (r.is_valid()) {
    //         num_completions = m_unsorted_docs_list.topk(r, k,
    //         m_pool.scores());
    //     }

    //     if (num_completions < k) {
    //         if (num_terms == 1) {  // special case
    //             suffix_lex_range.begin -= 1;
    //             num_completions = m_unsorted_minimal_docs_list.topk(
    //                 suffix_lex_range, k, m_pool.scores(),
    //                 true  // must return unique results
    //             );
    //             extract_completions(num_completions);
    //         } else {
    //             num_completions = conjunctive_topk(prefix, suffix_lex_range,
    //             k);
    //         }
    //     } else {
    //         extract_completions(num_completions);
    //     }

    //     return extract_strings(num_completions);
    // }

    size_t bytes() const {
        return m_completions.bytes() + m_unsorted_docs_list.bytes() +
               m_unsorted_minimal_docs_list.bytes() + m_dictionary.bytes() +
               m_docid_to_lexid.bytes() + m_inverted_index.bytes();
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
    unsorted_list_type m_unsorted_docs_list;
    typedef minimal_docids<cartesian_tree, InvertedIndex> minimal_docids_type;
    minimal_docids_type m_unsorted_minimal_docs_list;
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

    uint32_t conjunctive_topk(completion_type& prefix, const range suffix,
                              uint32_t const k) {
        deduplicate(prefix);
        if (prefix.size() == 1) {  // we've got nothing to intersect
            auto it = m_inverted_index.iterator(prefix.front() - 1);
            return conjunctive_topk(it, suffix, k);
        }
        auto it = m_inverted_index.intersection_iterator(prefix);
        return conjunctive_topk(it, suffix, k);
    }

    template <typename Iterator>
    uint32_t conjunctive_topk(Iterator& it, const range r, const uint32_t k) {
        auto& topk_scores = m_pool.scores();
        auto& completions = m_topk_completion_set.completions();
        auto& sizes = m_topk_completion_set.sizes();
        uint32_t i = 0;

        for (; it.has_next(); ++it) {
            auto doc_id = *it;
            auto lex_id = m_docid_to_lexid[doc_id];
            uint32_t size = m_completions.extract(lex_id, completions[i]);
            for (uint32_t j = 0; j != size; ++j) {
                if (r.contains(completions[i][j])) {
                    topk_scores[i] = doc_id;
                    sizes[i] = size;
                    ++i;
                    if (i == k) return k;
                    break;
                }
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