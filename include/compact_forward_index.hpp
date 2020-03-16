#pragma once

#include "parameters.hpp"
#include "compact_vector.hpp"
#include "ef/ef_sequence.hpp"

namespace autocomplete {

struct compact_forward_index {
    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_num_integers(0)
            , m_num_terms(params.num_terms) {
            essentials::logger("building forward_index...");
            uint64_t universe = params.universe;
            std::ifstream input(
                (params.collection_basename + ".forward").c_str(),
                std::ios_base::in);
            std::vector<id_type> terms;
            terms.reserve(universe *
                          constants::MAX_NUM_TERMS_PER_QUERY);  // at most
            uint64_t size = 0;
            m_pointers.push_back(0);
            for (uint64_t i = 0; i != universe; ++i) {
                uint32_t n = 0;
                input >> n;
                assert(n < constants::MAX_NUM_TERMS_PER_QUERY);
                m_num_integers += n;
                size += n;
                for (uint64_t k = 0; k != n; ++k) {
                    id_type x;
                    input >> x;
                    assert(x > 0);
                    terms.push_back(x);
                }
                m_pointers.push_back(size);
            }
            input.close();
            m_data.resize(terms.size(), util::ceil_log2(m_num_terms + 1));
            m_data.fill(terms.begin(), terms.size());
            essentials::logger("DONE");
        }

        void swap(compact_forward_index::builder& other) {
            std::swap(other.m_num_integers, m_num_integers);
            std::swap(other.m_num_terms, m_num_terms);
            other.m_pointers.swap(m_pointers);
            other.m_data.swap(m_data);
        }

        void build(compact_forward_index& fi) {
            fi.m_num_integers = m_num_integers;
            fi.m_num_terms = m_num_terms;
            fi.m_pointers.build(m_pointers);
            m_data.build(fi.m_data);
            builder().swap(*this);
        }

    private:
        uint64_t m_num_integers;
        uint64_t m_num_terms;
        std::vector<uint64_t> m_pointers;
        compact_vector::builder m_data;
    };

    compact_forward_index() {}

    struct forward_list_iterator_type {
        forward_list_iterator_type(compact_vector const& cv, uint64_t pos,
                                   uint64_t n)
            : m_cv(cv)
            , m_base(pos)
            , m_n(n)
            , m_i(0) {}

        uint64_t size() const {
            return m_n;
        }

        void operator++() {
            m_i += 1;
        }

        id_type operator*() const {
            return m_cv[m_base + m_i];
        }

        bool intersects(const range r) const {
            for (uint64_t i = 0; i != size(); ++i) {
                auto val = m_cv[m_base + i];
                assert(val > 0);
                if (r.contains(val)) return true;
            }
            return false;
        }

    private:
        compact_vector const& m_cv;
        uint64_t m_base;
        uint64_t m_n;
        uint64_t m_i;
    };

    forward_list_iterator_type iterator(id_type doc_id) {
        assert(doc_id < num_docs());
        uint64_t pos = m_pointers.access(doc_id);
        uint64_t n = m_pointers.access(doc_id + 1) - pos;
        return {m_data, pos, n};
    }

    bool intersects(const id_type doc_id, const range r) {
        return iterator(doc_id).intersects(r);
    }

    uint64_t num_integers() const {
        return m_num_integers;
    }

    uint64_t num_terms() const {
        return m_num_terms;
    }

    uint64_t num_docs() const {
        return m_pointers.size() - 1;
    }

    size_t data_bytes() const {
        return m_data.bytes();
    }

    size_t pointer_bytes() const {
        return m_pointers.bytes();
    }

    size_t bytes() const {
        return essentials::pod_bytes(m_num_integers) +
               essentials::pod_bytes(m_num_terms) + m_pointers.bytes() +
               m_data.bytes();
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_num_integers);
        visitor.visit(m_num_terms);
        visitor.visit(m_pointers);
        visitor.visit(m_data);
    }

private:
    uint64_t m_num_integers;
    uint64_t m_num_terms;
    ef::ef_sequence m_pointers;
    compact_vector m_data;
};

}  // namespace autocomplete