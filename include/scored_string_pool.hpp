#pragma once

#include "util_types.hpp"

namespace autocomplete {

struct scored_byte_range {
    byte_range string;
    id_type score;
};

struct scored_string_pool {
    void init() {
        push_back_offset(0);
    }

    void resize(size_t num_bytes, uint32_t k) {
        m_scores.resize(k);
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

    std::vector<id_type>& scores() {
        return m_scores;
    }

    std::vector<id_type> const& const_scores() const {
        return m_scores;
    }

    scored_byte_range operator[](size_t i) const {
        assert(i < size());
        scored_byte_range sbr;
        sbr.string = {m_data.data() + m_offsets[i],
                      m_data.data() + m_offsets[i + 1]};
        sbr.score = m_scores[i];
        return sbr;
    }

    struct iterator {
        iterator(scored_string_pool const* pool, size_t pos = 0)
            : m_pool(pool)
            , m_pos(pos) {}

        void operator++() {
            ++m_pos;
        }

        bool empty() const {
            return size() == 0;
        }

        size_t size() const {
            return m_pool->size();
        }

        scored_byte_range operator*() {
            return m_pool->operator[](m_pos);
        }

        scored_string_pool const* pool() const {
            return m_pool;
        }

    private:
        scored_string_pool const* m_pool;
        size_t m_pos;
    };

    iterator begin() {
        return iterator(this);
    }

private:
    std::vector<id_type> m_scores;
    std::vector<size_t> m_offsets;
    std::vector<uint8_t> m_data;
};

}  // namespace autocomplete