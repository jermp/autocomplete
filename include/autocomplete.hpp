#pragma once

#include "util_types.hpp"
#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"
#include "constants.hpp"

namespace autocomplete {

template <typename Completions, typename Dictionary, typename InvertedIndex,
          typename ForwardIndex>
struct autocomplete {
    typedef scored_string_pool::iterator iterator_type;

    autocomplete() {
        m_pool.resize(constants::POOL_SIZE, constants::MAX_K);
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
    //         } else {
    //             num_completions = conjunctive_topk(prefix, suffix_lex_range,
    //             k);
    //         }
    //     }

    //     return extract_strings(num_completions);
    // }

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
    unsorted_list_type m_unsorted_docs_list;
    typedef minimal_docids<cartesian_tree, InvertedIndex> minimal_docids_type;
    minimal_docids_type m_unsorted_minimal_docs_list;
    Dictionary m_dictionary;
    InvertedIndex m_inverted_index;
    ForwardIndex m_forward_index;

    scored_string_pool m_pool;

    void init() {
        m_pool.clear();
        m_pool.init();
        assert(m_pool.size() == 0);
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
    uint32_t conjunctive_topk(Iterator& it, const range r, uint32_t const k) {
        auto& topk_scores = m_pool.scores();
        uint32_t results = 0;
        for (; it.has_next(); ++it) {
            auto doc_id = *it;
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
            auto doc_id = topk_scores[i];
            auto it = m_forward_index.iterator(doc_id);
            uint64_t offset = m_pool.bytes();
            uint8_t* decoded = m_pool.data() + offset;
            for (uint32_t j = 0; j != it.size(); ++j, ++it) {
                auto term_id = *it;
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