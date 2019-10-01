#pragma once

#include "parameters.hpp"

namespace autocomplete {

template <typename ListType, typename Pointers>
struct inverted_index {
    typedef typename ListType::iterator iterator_type;

    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_num_docs(params.num_completions) {
            essentials::logger("building inverted_index...");

            uint64_t num_terms = params.num_terms;
            m_minimal_doc_ids.reserve(num_terms);

            std::ifstream input(
                (params.collection_basename + ".inverted").c_str(),
                std::ios_base::in);

            std::vector<id_type> list;
            m_pointers.push_back(0);
            for (uint64_t i = 0; i != num_terms; ++i) {
                list.clear();
                uint32_t n = 0;
                input >> n;
                list.reserve(n);
                for (uint64_t k = 0; k != n; ++k) {
                    id_type x;
                    input >> x;
                    list.push_back(x);
                }
                m_minimal_doc_ids.push_back(list.front());
                m_bvb.append_bits(n, 32);
                ListType::build(m_bvb, list.begin(), list.size());
                m_pointers.push_back(m_bvb.size());
            }

            m_pointers.pop_back();
            input.close();
            essentials::logger("DONE");
        }

        std::vector<id_type>& minimal_doc_ids() {
            return m_minimal_doc_ids;
        }

        void swap(inverted_index::builder& other) {
            std::swap(other.m_num_docs, m_num_docs);
            other.m_pointers.swap(m_pointers);
            other.m_minimal_doc_ids.swap(m_minimal_doc_ids);
            other.m_bvb.swap(m_bvb);
        }

        void build(inverted_index<ListType, Pointers>& ii) {
            ii.m_num_docs = m_num_docs;
            ii.m_pointers.build(m_pointers);
            ii.m_data.build(&m_bvb);
            builder().swap(*this);
        }

    private:
        uint64_t m_num_docs;
        std::vector<uint64_t> m_pointers;
        std::vector<id_type> m_minimal_doc_ids;
        bit_vector_builder m_bvb;
    };

    inverted_index() {}

    iterator_type iterator(id_type term_id) const {
        uint64_t offset = m_pointers.access(term_id);
        uint32_t n = m_data.get_bits(offset, 32);
        iterator_type it(m_data, offset + 32, m_num_docs, n, m_params);
        return it;
    }

    uint64_t num_terms() const {
        return m_pointers.size();
    }

    uint64_t num_docs() const {
        return m_num_docs;
    }

    size_t bytes() const {
        return essentials::pod_bytes(m_num_docs) + m_pointers.bytes() +
               m_data.bytes();
    }

    // uint32_t intersect(std::vector<id_type> const& term_ids,
    //                    std::vector<id_type>& out) {
    //     assert(term_ids.size() > 0);

    //     if (term_ids.size() == 1) {
    //         auto it = iterator(term_ids.front() - 1);
    //         return it.decode(out.data());
    //     }

    //     static std::vector<iterator_type> iterators;
    //     iterators.clear();
    //     iterators.reserve(term_ids.size());

    //     for (auto id : term_ids) {
    //         assert(id > 0);  // id 0 is reserved for null terminator
    //         iterators.push_back(std::move(iterator(id - 1)));
    //     }

    //     std::sort(
    //         iterators.begin(), iterators.end(),
    //         [](auto const& l, auto const& r) { return l.size() < r.size();
    //         });

    //     uint32_t size = 0;
    //     id_type candidate = iterators[0].access(0);
    //     size_t i = 1;
    //     while (candidate < num_docs()) {
    //         for (; i < iterators.size(); ++i) {
    //             id_type val = iterators[i].next_geq(candidate);
    //             if (val != candidate) {
    //                 candidate = val;
    //                 i = 0;
    //                 break;
    //             }
    //         }

    //         if (i == iterators.size()) {
    //             out[size++] = candidate;
    //             candidate = iterators[0].next();
    //             i = 1;
    //         }
    //     }

    //     return size;
    // }

    struct intersection_iterator_type {
        intersection_iterator_type(inverted_index const* ii,
                                   std::vector<id_type> const& term_ids) {
            assert(term_ids.size() > 1);
            m_iterators.reserve(term_ids.size());
            for (auto id : term_ids) {
                assert(id > 0);  // id 0 is reserved for null terminator
                m_iterators.push_back(std::move(ii->iterator(id - 1)));
            }

            std::sort(m_iterators.begin(), m_iterators.end(),
                      [](auto const& l, auto const& r) {
                          return l.size() < r.size();
                      });

            m_candidate = m_iterators[0].access(0);
            m_i = 1;
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
            m_candidate = m_iterators[0].next();
            m_i = 1;
            next();
        }

    private:
        id_type m_candidate;
        size_t m_i;
        uint64_t m_num_docs;
        std::vector<iterator_type> m_iterators;

        void next() {
            id_type val = 0;
            while (val < m_num_docs and m_i != m_iterators.size()) {
                val = m_iterators[m_i].next_geq(m_candidate);
                if (val != m_candidate) {
                    m_candidate = val;
                    m_i = 0;
                } else {
                    ++m_i;
                }
            }
        }
    };

    intersection_iterator_type intersection_iterator(
        std::vector<id_type> const& term_ids) {
        return intersection_iterator_type(this, term_ids);
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_num_docs);
        visitor.visit(m_pointers);
        visitor.visit(m_data);
    }

private:
    uint64_t m_num_docs;
    compression_parameters m_params;
    Pointers m_pointers;
    bit_vector m_data;
};

}  // namespace autocomplete