#pragma once

#include <vector>
#include <algorithm>

namespace autocomplete {

template <typename Iterator>
struct iterator_comparator {
    bool operator()(Iterator& l, Iterator& r) {
        return l.operator*() > r.operator*();
    }
};

template <typename T, typename Comparator>
struct min_heap {
    void reserve(uint64_t n) {
        m_q.reserve(n);
    }

    T& top() {
        return m_q.front();
    }

    T& operator[](uint64_t i) {
        assert(i < size());
        return m_q[i];
    }

    void push(T const& t) {
        m_q.push_back(t);
        std::push_heap(m_q.begin(), m_q.end(), m_comparator);
    }

    void pop() {
        std::pop_heap(m_q.begin(), m_q.end(), m_comparator);
        m_q.pop_back();
    }

    void push_back(T const& t) {
        m_q.push_back(t);
    }

    void make_heap() {
        std::make_heap(m_q.begin(), m_q.end(), m_comparator);
    }

    void heapify() {
        sink(0);
    }

    void clear() {
        m_q.clear();
    }

    bool empty() const {
        return m_q.empty();
    }

    inline uint64_t size() const {
        return m_q.size();
    }

private:
    std::vector<T> m_q;
    Comparator m_comparator;

    void sink(uint64_t pos) {
        assert(pos <= size());
        while (2 * pos + 1 < size()) {
            uint64_t i = 2 * pos + 1;
            if (i + 1 < size() and m_comparator(m_q[i], m_q[i + 1])) ++i;
            if (!m_comparator(m_q[pos], m_q[i])) break;
            std::swap(m_q[pos], m_q[i]);
            pos = i;
        }
    }
};
}  // namespace autocomplete
