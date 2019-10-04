#pragma once

#include "autocomplete_common.hpp"
#include "scored_string_pool.hpp"

namespace autocomplete {

template <typename Completions, typename UnsortedDocsList, typename Dictionary,
          typename InvertedIndex>
struct autocomplete2 {
    typedef scored_string_pool::iterator iterator_type;

    autocomplete2() {
        m_pool.resize(POOL_SIZE, MAX_K);
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

    iterator_type prefix_topk(std::string& query, uint32_t k) {
        assert(k <= MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        parse(m_dictionary, query, prefix, suffix);

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;

        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
        uint32_t num_completions =
            m_unsorted_docs_list.topk(r, k, m_pool.scores());

        extract_completions(num_completions);

        return extract_strings(num_completions);
    }

    iterator_type conjunctive_topk(std::string& query, uint32_t k) {
        assert(k <= MAX_K);
        init();
        completion_type prefix;
        byte_range suffix;
        uint32_t num_terms = parse(m_dictionary, query, prefix, suffix);
        assert(num_terms > 0);

        uint32_t num_completions = 0;

        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);

        if (num_terms == 1) {  // special case

            suffix_lex_range.end += 1;
            num_completions = m_unsorted_minimal_docs_list.topk(
                suffix_lex_range, k, m_pool.scores(),
                true  // must return unique results
            );

        } else {
            if (prefix.size() == 1) {  // we've got nothing to intersect
                auto it = m_inverted_index.iterator(prefix.front() - 1);
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            }
        }

        return extract_strings(num_completions);
    }

    // for benchmarking
    iterator_type prefix_topk(std::string& query, uint32_t k,
                              std::vector<timer_type>& timers) {
        // step 0
        timers[0].start();
        assert(k <= MAX_K);
        init();
        completion_type prefix;
        byte_range suffix{0, 0};
        parse(m_dictionary, query, prefix, suffix);
        timers[0].stop();

        // step 1
        timers[1].start();
        suffix.end += 1;  // include null terminator
        range suffix_lex_range = m_dictionary.locate_prefix(suffix);
        suffix_lex_range.begin += 1;
        suffix_lex_range.end += 1;
        range r = m_completions.locate_prefix(prefix, suffix_lex_range);
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
    iterator_type conjunctive_topk(std::string& query, uint32_t k,
                                   std::vector<timer_type>& timers) {
        // step 0
        timers[0].start();
        assert(k <= MAX_K);
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
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
            } else {
                auto it = m_inverted_index.intersection_iterator(prefix);
                num_completions = conjunctive_topk(it, suffix_lex_range, k);
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

    void init() {
        m_pool.clear();
        m_pool.init();
    }

    void extract_completions(uint32_t num_completions) {
        auto& topk = m_pool.scores();
        auto& completions = m_pool.completions();
        for (uint32_t i = 0; i != num_completions; ++i) {
            id_type doc_id = topk[i];
            static completion_type c(128);
            id_type lex_id = m_docid_to_lexid[doc_id];
            uint32_t size = m_completions.extract(lex_id, c);

            // is this expensive? is memcpy faster (with prior resizing)?
            for (uint32_t k = 0; k != size; ++k) {
                completions[i].push_back(c[k]);
            }
        }
    }

    template <typename Iterator>
    uint32_t conjunctive_topk(Iterator& it, range r, uint32_t k) {
        auto& topk = m_pool.scores();
        auto& completions = m_pool.completions();
        uint32_t i = 0;
        static completion_type c(128);

        while (it.has_next()) {
            id_type doc_id = *it;

            bool match = false;
            id_type lex_id = m_docid_to_lexid[doc_id];
            uint32_t size = m_completions.extract(lex_id, c);
            for (uint32_t j = 0; j != size; ++j) {
                if (c[j] >= r.begin and c[j] <= r.end) match = true;
            }

            if (match) {
                topk[i] = doc_id;

                // is this expensive? is memcpy faster (with prior resizing)?
                for (uint32_t k = 0; k != size; ++k) {
                    completions[i].push_back(c[k]);
                }

                ++i;
                if (i == k) break;
            }
            ++it;
        }
        return i;
    }

    iterator_type extract_strings(uint32_t num_completions) {
        auto const& completions = m_pool.completions();
        for (uint32_t i = 0; i != num_completions; ++i) {
            auto const& c = completions[i];
            uint64_t offset = m_pool.bytes();
            uint8_t* decoded = m_pool.data() + offset;
            for (uint32_t j = 0; j != c.size(); ++j) {
                id_type term_id = c[j];
                uint8_t len = m_dictionary.extract(term_id, decoded);
                decoded += len;
                offset += len;
                if (j != c.size() - 1) {
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