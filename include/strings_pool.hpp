#pragma once

#include "util_types.hpp"

namespace autocomplete {

struct strings_pool {
    void init() {
        push_back_offset(0);
    }

    void resize(size_t num_bytes) {
        m_data.resize(num_bytes);
    }

    void clear() {
        m_offsets.clear();
    }

    size_t size() const {
        assert(m_offsets.size() > 0);
        return m_offsets.size() - 1;
    }

    size_t bytes() const {
        return m_offsets.back();
    }

    uint8_t* data() {
        return m_data.data();
    }

    void push_back_offset(size_t offset) {
        m_offsets.push_back(offset);
    }

    byte_range operator[](size_t i) const {
        assert(i < size());
        return {m_data.data() + m_offsets[i], m_data.data() + m_offsets[i + 1]};
    }

    struct iterator {
        iterator(strings_pool const* pool, size_t pos = 0)
            : m_pool(pool)
            , m_pos(pos) {}

        void operator++() {
            ++m_pos;
        }

        size_t size() const {
            return m_pool->size();
        }

        byte_range operator*() {
            return m_pool->operator[](m_pos);
        }

    private:
        strings_pool const* m_pool;
        size_t m_pos;
    };

    iterator begin() {
        return iterator(this);
    }

private:
    std::vector<size_t> m_offsets;
    std::vector<uint8_t> m_data;
};

}  // namespace autocomplete