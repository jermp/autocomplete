#pragma once

#include <algorithm>
#include <unordered_map>
#include "parameters.hpp"

namespace autocomplete {

template <typename InvertedListType, typename OffsetListType,
          typename TermListType, typename Pointers>
struct blocked_inverted_index {
    typedef typename InvertedListType::iterator docs_iterator_type;
    typedef typename OffsetListType::iterator offsets_iterator_type;
    typedef typename TermListType::iterator terms_iterator_type;

    struct builder {
        builder() {}

        builder(parameters const& params, float c)
            : m_num_docs(params.num_completions)
            , m_num_terms(params.num_terms) {
            essentials::logger("building blocked_inverted_index with c = " +
                               std::to_string(c) + "...");

            uint64_t num_terms = params.num_terms;
            m_minimal_doc_ids.reserve(num_terms);

            uint64_t m = m_num_docs * c;

            std::ifstream input(
                (params.collection_basename + ".inverted").c_str(),
                std::ios_base::in);

            m_pointers_to_lists.push_back(0);
            m_pointers_to_offsets.push_back(0);
            m_pointers_to_terms.push_back(0);

            std::unordered_map<id_type, std::vector<id_type>> terms;
            terms.reserve(m_num_docs);
            for (uint64_t doc_id = 0; doc_id != m_num_docs; ++doc_id) {
                terms[doc_id] = std::vector<id_type>();
            }

            std::vector<id_type> union_of_lists;
            union_of_lists.reserve(m);
            uint64_t num_postings_in_block = 0;

            for (uint64_t term_id = 1; term_id <= num_terms; ++term_id) {
                uint32_t n = 0;
                input >> n;
                num_postings_in_block += n;

                for (uint64_t k = 0; k != n; ++k) {
                    id_type doc_id;
                    input >> doc_id;
                    union_of_lists.push_back(doc_id);
                    terms[doc_id].push_back(term_id);
                    if (k == 0) {
                        m_minimal_doc_ids.push_back(doc_id);
                    }
                }

                if (num_postings_in_block >= m or term_id == num_terms) {
                    m_blocks.push_back(term_id);
                    std::sort(union_of_lists.begin(), union_of_lists.end());
                    auto end = std::unique(union_of_lists.begin(),
                                           union_of_lists.end());
                    uint64_t size = std::distance(union_of_lists.begin(), end);
                    std::cout << "union has size " << size << std::endl;

                    m_lists.append_bits(size, 32);
                    InvertedListType::build(m_lists, union_of_lists.begin(),
                                            size);
                    m_pointers_to_lists.push_back(m_lists.size());

                    std::vector<id_type> term_list;
                    std::vector<uint64_t> offset_list;

                    uint64_t offset = 0;
                    for (uint64_t i = 0; i != size; ++i) {
                        auto doc_id = union_of_lists[i];
                        auto& set_of_term_ids = terms[doc_id];
                        assert(std::is_sorted(set_of_term_ids.begin(),
                                              set_of_term_ids.end()));
                        offset_list.push_back(offset);

                        for (auto t : set_of_term_ids) {
                            term_list.push_back(t);
                        }

                        offset += set_of_term_ids.size();
                        set_of_term_ids.clear();
                    }

                    offset_list.push_back(offset);

                    m_offsets.append_bits(offset_list.size(), 32);
                    m_offsets.append_bits(offset_list.back() + 1, 32);
                    OffsetListType::build(m_offsets, offset_list.begin(),
                                          offset_list.size());
                    m_pointers_to_offsets.push_back(m_offsets.size());

                    m_terms.append_bits(term_list.size(), 32);
                    TermListType::build(m_terms, term_list.begin(),
                                        term_list.size());
                    m_pointers_to_terms.push_back(m_terms.size());

                    num_postings_in_block = 0;
                    union_of_lists.clear();
                }
            }

            input.close();

            m_pointers_to_lists.pop_back();
            m_pointers_to_offsets.pop_back();
            m_pointers_to_terms.pop_back();

            essentials::logger("DONE");
        }

        std::vector<id_type>& minimal_doc_ids() {
            return m_minimal_doc_ids;
        }

        void swap(blocked_inverted_index::builder& other) {
            std::swap(other.m_num_docs, m_num_docs);
            std::swap(other.m_num_terms, m_num_terms);

            other.m_blocks.swap(m_blocks);

            other.m_pointers_to_lists.swap(m_pointers_to_lists);
            other.m_pointers_to_offsets.swap(m_pointers_to_offsets);
            other.m_pointers_to_terms.swap(m_pointers_to_terms);

            other.m_lists.swap(m_lists);
            other.m_offsets.swap(m_offsets);
            other.m_terms.swap(m_terms);

            other.m_minimal_doc_ids.swap(m_minimal_doc_ids);
        }

        void build(blocked_inverted_index<InvertedListType, OffsetListType,
                                          TermListType, Pointers>& ii) {
            ii.m_num_docs = m_num_docs;
            ii.m_num_terms = m_num_terms;

            ii.m_blocks.swap(m_blocks);

            ii.m_pointers_to_lists.build(m_pointers_to_lists);
            ii.m_pointers_to_offsets.build(m_pointers_to_offsets);
            ii.m_pointers_to_terms.build(m_pointers_to_terms);

            ii.m_lists.build(&m_lists);
            ii.m_offsets.build(&m_offsets);
            ii.m_terms.build(&m_terms);

            builder().swap(*this);
        }

    private:
        uint64_t m_num_docs;
        uint64_t m_num_terms;

        std::vector<uint32_t> m_blocks;

        std::vector<uint64_t> m_pointers_to_lists;
        std::vector<uint64_t> m_pointers_to_offsets;
        std::vector<uint64_t> m_pointers_to_terms;

        bit_vector_builder m_lists;
        bit_vector_builder m_offsets;
        bit_vector_builder m_terms;

        std::vector<id_type> m_minimal_doc_ids;
    };

    blocked_inverted_index() {}

    uint64_t num_terms() const {
        return m_num_terms;
    }

    uint64_t num_docs() const {
        return m_num_docs;
    }

    uint64_t num_blocks() const {
        return m_blocks.size();
    }

    size_t bytes() const {
        return essentials::pod_bytes(m_num_docs) + m_pointers_to_lists.bytes() +
               m_lists.bytes() + m_pointers_to_offsets.bytes() +
               m_offsets.bytes() + m_pointers_to_terms.bytes() +
               m_terms.bytes();
    }

    uint32_t block_id(id_type term_id) const {
        uint32_t id = 0;
        for (; id != num_blocks(); ++id) {
            uint32_t block_size = m_blocks[id];
            if (term_id < block_size) break;
        }
        return id;
    }

    struct block_type {
        docs_iterator_type docs_iterator;
        offsets_iterator_type offsets_iterator;
        terms_iterator_type terms_iterator;
        std::vector<id_type> term_ids;
    };

    struct intersection_iterator_type {
        intersection_iterator_type(blocked_inverted_index const* ii,
                                   std::vector<id_type>& term_ids,
                                   const range r)
            : m_suffix(r) {
            assert(!is_invalid(r));

            if (!term_ids.empty()) {
                m_iterators.reserve(term_ids.size());  // at most
                std::sort(term_ids.begin(), term_ids.end());
                uint32_t current_block_id = ii->block_id(term_ids.front());
                uint32_t i = 0;
                uint32_t prev_i = 0;
                for (; i != term_ids.size(); ++i) {
                    auto term_id = term_ids[i];
                    assert(term_id > 0);
                    uint32_t b = ii->block_id(term_id);
                    if (b > current_block_id) {
                        auto block = ii->block(current_block_id);
                        block.term_ids.reserve(term_ids.size());  // at most
                        for (; prev_i != i; ++prev_i) {
                            block.term_ids.push_back(term_ids[prev_i]);
                        }
                        m_iterators.push_back(std::move(block));
                    }

                    current_block_id = b;
                }

                auto block = ii->block(current_block_id);
                block.term_ids.reserve(term_ids.size());  // at most
                for (; prev_i != i; ++prev_i) {
                    block.term_ids.push_back(term_ids[prev_i]);
                }
                m_iterators.push_back(std::move(block));

                assert(m_iterators.size() > 0);
                std::sort(m_iterators.begin(), m_iterators.end(),
                          [](auto const& l, auto const& r) {
                              return l.docs_iterator.size() <
                                     r.docs_iterator.size();
                          });

                m_candidate = m_iterators[0].docs_iterator.access(0);
            } else {
                m_candidate = 0;
            }

            {
                uint32_t current_block_id = ii->block_id(r.begin);
                uint32_t i = r.begin;
                for (; i != r.end; ++i) {
                    assert(i > 0);
                    uint32_t b = ii->block_id(i);
                    if (b > current_block_id) {
                        m_range.push_back(ii->block(current_block_id));
                    }
                    current_block_id = b;
                }
                m_range.push_back(ii->block(current_block_id));
            }

            m_i = 0;
            m_num_docs = ii->num_docs();
            next();
        }

        bool has_next() const {
            return m_candidate < m_num_docs;
        }

        id_type operator*() {
            return m_candidate;
        }

        void operator++() {
            assert(m_i == m_iterators.size());
            if (!m_iterators.empty()) {
                if (m_iterators.size() > 1) {
                    m_candidate = m_iterators[0].docs_iterator.next();
                }
            } else {
                m_candidate += 1;
            }
            m_i = 0;
            next();
        }

        bool intersects() {
            for (auto& block : m_range) {
                uint64_t val = block.docs_iterator.next_geq(m_candidate);
                if (val == m_candidate) {
                    uint64_t pos = block.docs_iterator.position();
                    assert(block.docs_iterator.access(pos) == m_candidate);
                    uint64_t begin = block.offsets_iterator.access(pos);
                    uint64_t end = block.offsets_iterator.access(pos + 1);
                    assert(end > begin);
                    for (uint64_t i = begin; i != end; ++i) {
                        assert(i < block.terms_iterator.size());
                        auto t = block.terms_iterator.access(i);
                        if (t > m_suffix.end) break;
                        if (t >= m_suffix.begin and t <= m_suffix.end) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

    private:
        id_type m_candidate;
        size_t m_i;
        uint64_t m_num_docs;
        std::vector<block_type> m_iterators;
        std::vector<block_type> m_range;
        range m_suffix;

        bool in() {  // is candidate doc in intersection?

            uint64_t pos = m_iterators[m_i].docs_iterator.position();
            if (pos == m_iterators[m_i].docs_iterator.size()) return false;
            uint64_t begin = m_iterators[m_i].offsets_iterator.access(pos);
            uint64_t end = m_iterators[m_i].offsets_iterator.access(pos + 1);
            assert(end > begin);
            uint64_t size = end - begin;
            if (size < m_iterators[m_i].term_ids.size()) return false;

            uint64_t i = begin;
            for (auto x : m_iterators[m_i].term_ids) {
                bool found = false;
                for (; i != end; ++i) {
                    auto t = m_iterators[m_i].terms_iterator.access(i);
                    if (t == x) {
                        found = true;
                        break;
                    }
                }
                if (!found) return false;
            }

            return true;
        }

        void next() {
            if (m_iterators.size() == 1) {
                while (m_candidate < m_num_docs and m_i != m_iterators.size()) {
                    assert(m_i == 0);
                    m_candidate = m_iterators[m_i].docs_iterator.next();
                    if (in()) ++m_i;
                }
            } else {
                while (m_candidate < m_num_docs and m_i != m_iterators.size()) {
                    // NOTE: since we work with (set) unions of posting lists,
                    // next_geq by scan is considerably faster than a
                    // traditional binary search
                    auto val = m_iterators[m_i].docs_iterator.next_geq_by_scan(
                        m_candidate);
                    bool is_in = in();
                    if (val == m_candidate and is_in) {
                        ++m_i;
                    } else {
                        m_candidate = val + !is_in;
                        m_i = 0;
                    }
                }
            }
        }
    };

    intersection_iterator_type intersection_iterator(
        std::vector<id_type>& term_ids, const range r) {
        return intersection_iterator_type(this, term_ids, r);
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_num_docs);
        visitor.visit(m_num_terms);
        visitor.visit(m_blocks);
        visitor.visit(m_pointers_to_lists);
        visitor.visit(m_lists);
        visitor.visit(m_pointers_to_offsets);
        visitor.visit(m_offsets);
        visitor.visit(m_pointers_to_terms);
        visitor.visit(m_terms);
    }

private:
    uint64_t m_num_docs;
    uint64_t m_num_terms;
    compression_parameters m_params;
    std::vector<uint32_t> m_blocks;
    Pointers m_pointers_to_lists;
    bit_vector m_lists;
    Pointers m_pointers_to_offsets;
    bit_vector m_offsets;
    Pointers m_pointers_to_terms;
    bit_vector m_terms;

    block_type block(uint32_t i) const {
        assert(i < num_blocks());

        block_type b;

        {
            uint64_t offset = m_pointers_to_lists.access(i);
            uint32_t n = m_lists.get_bits(offset, 32);
            docs_iterator_type it(m_lists, offset + 32, m_num_docs, n,
                                  m_params);
            b.docs_iterator = it;
        }

        {
            uint64_t offset = m_pointers_to_offsets.access(i);
            uint32_t n = m_offsets.get_bits(offset, 32);
            uint32_t universe = m_offsets.get_bits(offset + 32, 32);
            offsets_iterator_type it(m_offsets, offset + 32 + 32, universe, n,
                                     m_params);
            b.offsets_iterator = it;
        }

        {
            uint64_t offset = m_pointers_to_terms.access(i);
            uint32_t n = m_terms.get_bits(offset, 32);
            terms_iterator_type it(m_terms, offset + 32, m_num_terms, n,
                                   m_params);
            b.terms_iterator = it;
        }

        return b;
    }
};

}  // namespace autocomplete