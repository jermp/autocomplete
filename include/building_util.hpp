#pragma once

#include "bit_vector.hpp"

namespace autocomplete {
namespace util {

void push_pad(bit_vector_builder& bvb, uint64_t alignment = 8) {
    uint64_t mod = bvb.size() % alignment;
    if (mod) {
        uint64_t pad = alignment - mod;
        bvb.append_bits(0, pad);
        assert(bvb.size() % alignment == 0);
    }
}

void eat_pad(bits_iterator<bit_vector>& it, uint64_t alignment = 8) {
    uint64_t mod = it.position() % alignment;
    if (mod) {
        uint64_t pad = alignment - mod;
        it.get_bits(pad);
        assert(it.position() % alignment == 0);
    }
}

template <typename Iterator>
struct first_iterator
    : std::iterator<std::forward_iterator_tag,
                    typename Iterator::value_type::first_type> {
    first_iterator(Iterator it, uint64_t state = 0)
        : m_it(it)
        , m_state(state) {}

    typename Iterator::value_type::first_type operator*() {
        return (*m_it).first;
    }

    first_iterator& operator++() {
        m_it += 1;
        m_state += 1;
        return *this;
    }

    first_iterator operator+(uint64_t n) {
        return {m_it + n, m_state + n};
    }

    bool operator==(first_iterator const& other) const {
        return m_state == other.m_state;
    }

    bool operator!=(first_iterator const& other) const {
        return !(*this == other);
    }

private:
    Iterator m_it;
    uint64_t m_state;
};

}  // namespace util
}  // namespace autocomplete