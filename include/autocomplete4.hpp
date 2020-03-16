#pragma once

#include "util_types.hpp"
#include "building_util.hpp"
#include "compact_vector.hpp"
#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"
#include "constants.hpp"

namespace autocomplete {

/* Bast and Weber approach. */

template <typename Completions, typename Dictionary,
          typename BlockedInvertedIndex>
struct autocomplete4 {
    typedef scored_string_pool::iterator iterator_type;

    autocomplete4() {
        m_pool.resize(constants::POOL_SIZE, constants::MAX_K);
        m_topk_completion_set.resize(constants::MAX_K,
                                     2 * constants::MAX_NUM_TERMS_PER_QUERY);
    }

    autocomplete4(parameters const& params, float c)
        : autocomplete4() {
        typename Completions::builder cm_builder(params);
        typename Dictionary::builder di_builder(params);
        typename BlockedInvertedIndex::builder ii_builder(params, c);
        auto const& docid_to_lexid = cm_builder.docid_to_lexid();
        m_docid_to_lexid.build(docid_to_lexid.begin(), docid_to_lexid.size(),
                               util::ceil_log2(params.num_completions + 1));
        m_unsorted_docs_list.build(
            util::invert(docid_to_lexid, params.num_completions));
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
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        uint32_t num_completions =
            conjunctive_topk(prefix, suffix_lex_range, k);
        probe.stop(1);

        probe.start(2);
        extract_completions(num_completions);
        auto it = extract_strings(num_completions);
        probe.stop(2);

        return it;
    }

    // iterator_type topk(std::string const& query, const uint32_t k) {
    //     assert(k <= constants::MAX_K);
    //     init();
    //     completion_type prefix;
    //     byte_range suffix;
    //     parse(m_dictionary, query, prefix, suffix);

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
    //         num_completions = conjunctive_topk(prefix, suffix_lex_range, k);
    //     }

    //     extract_completions(num_completions);
    //     return extract_strings(num_completions);
    // }

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
    unsorted_list_type m_unsorted_docs_list;
    Dictionary m_dictionary;
    BlockedInvertedIndex m_inverted_index;
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

    typedef typename BlockedInvertedIndex::block_type block_t;

    struct block_type_comparator {
        bool operator()(block_t& l, block_t& r) {
            return l.docs_iterator.operator*() > r.docs_iterator.operator*();
        }
    };

    uint32_t conjunctive_topk(completion_type& prefix, const range suffix,
                              const uint32_t k) {
        auto& topk_scores = m_pool.scores();

        typedef min_heap<block_t, block_type_comparator>
            min_priority_queue_type;
        min_priority_queue_type q;
        uint32_t current_block_id = m_inverted_index.block_id(suffix.begin);
        uint32_t current_block_boundary =
            m_inverted_index.block_boundary(current_block_id);
        for (uint32_t i = suffix.begin; i != suffix.end; ++i) {
            assert(i > 0);
            if (i > current_block_boundary) {
                q.push_back(m_inverted_index.block(current_block_id));
                current_block_id += 1;
                current_block_boundary =
                    m_inverted_index.block_boundary(current_block_id);
            }
        }
        q.push_back(m_inverted_index.block(current_block_id));
        q.make_heap();

        uint32_t results = 0;

        auto check = [&](block_t& block, id_type doc_id) {
            uint64_t pos = block.docs_iterator.position();
            assert(block.docs_iterator.access(pos) == doc_id);
            uint64_t begin = block.offsets_iterator.access(pos);
            uint64_t end = block.offsets_iterator.access(pos + 1);
            assert(end > begin);
            for (uint64_t i = begin; i != end; ++i) {
                auto t = block.terms_iterator.access(i) + block.lower_bound;
                if (t > suffix.end) break;
                if (suffix.contains(t)) {
                    topk_scores[results++] = doc_id;
                    break;
                }
            }
        };

        if (prefix.size() == 0) {
            while (!q.empty()) {
                auto& z = q.top();
                auto doc_id = z.docs_iterator.operator*();
                check(z, doc_id);
                if (results == k) return results;
                z.docs_iterator.next();
                if (!z.docs_iterator.has_next()) q.pop();
                q.heapify();
            }
        } else {
            deduplicate(prefix);
            auto it = m_inverted_index.intersection_iterator(prefix, suffix);
            for (; it.has_next() and !q.empty(); ++it) {
                auto doc_id = *it;
                while (!q.empty()) {
                    auto& z = q.top();
                    auto val = z.docs_iterator.operator*();
                    if (val > doc_id) break;
                    if (val < doc_id) {
                        val = z.docs_iterator.next_geq(doc_id);
                        if (!z.docs_iterator.has_next()) {
                            q.pop();
                        } else {
                            q.heapify();
                        }
                    } else {
                        if (val == doc_id) {
                            check(z, doc_id);
                            if (results == k) return results;
                        }
                        break;
                    }
                }
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