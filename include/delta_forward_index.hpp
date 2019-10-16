#pragma once

#include "parameters.hpp"
#include "bit_vector.hpp"
#include "ef/ef_sequence.hpp"

namespace autocomplete {

struct delta_forward_index {
    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_num_integers(0)
            , m_num_terms(params.num_terms) {
            essentials::logger("building forward_index...");
            uint64_t num_completions = params.num_completions;
            std::ifstream input(
                (params.collection_basename + ".forward").c_str(),
                std::ios_base::in);
            m_pointers.push_back(0);
            for (uint64_t i = 0; i != num_completions; ++i) {
                uint32_t n = 0;
                input >> n;
                assert(n > 0 and n < constants::MAX_NUM_TERMS_PER_QUERY);
                write_gamma_nonzero(m_data, n);
                m_num_integers += n;
                for (uint64_t k = 0; k != n; ++k) {
                    id_type x;
                    input >> x;
                    write_delta(m_data, x);
                }
                m_pointers.push_back(m_data.size());
            }
            m_pointers.pop_back();
            input.close();
            essentials::logger("DONE");
        }

        void swap(delta_forward_index::builder& other) {
            std::swap(other.m_num_integers, m_num_integers);
            std::swap(other.m_num_terms, m_num_terms);
            other.m_pointers.swap(m_pointers);
            other.m_data.swap(m_data);
        }

        void build(delta_forward_index& fi) {
            fi.m_num_integers = m_num_integers;
            fi.m_num_terms = m_num_terms;
            fi.m_pointers.build(m_pointers);
            fi.m_data.build(&m_data);
            builder().swap(*this);
        }

    private:
        uint64_t m_num_integers;
        uint64_t m_num_terms;
        std::vector<uint64_t> m_pointers;
        bit_vector_builder m_data;
    };

    delta_forward_index() {}

    struct forward_list_iterator_type {
        forward_list_iterator_type(bits_iterator<bit_vector> const& it,
                                   uint64_t n)
            : m_it(it)
            , m_n(n)
            , m_i(0) {}

        uint64_t size() const {
            return m_n;
        }

        void operator++() {
            m_i += 1;
        }

        id_type operator*() {
            return read_delta(m_it);
        }

        bool intersects(const range r) {
            for (uint64_t i = 0; i != size(); ++i) {
                auto val = operator*();
                if (r.contains(val)) return true;
            }
            return false;
        }

    private:
        bits_iterator<bit_vector> m_it;
        uint64_t m_n;
        uint64_t m_i;
    };

    forward_list_iterator_type iterator(id_type doc_id) {
        uint64_t offset = m_pointers.access(doc_id);
        bits_iterator<bit_vector> it(m_data, offset);
        uint64_t n = read_gamma_nonzero(it);
        return {it, n};
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
        return m_pointers.size();
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
    bit_vector m_data;
};

}  // namespace autocomplete