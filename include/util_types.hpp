#pragma once

#include <vector>
#include <functional>
#include <chrono>

#include "util.hpp"

namespace autocomplete {

typedef std::vector<id_type> completion_type;

struct completion_set {
    void resize(uint32_t k, uint32_t max_num_terms_per_completion) {
        m_sizes.resize(k);
        m_completions.resize(k);
        for (auto& c : m_completions) {
            c.resize(max_num_terms_per_completion);
        }
    }

    auto& completions() {
        return m_completions;
    }

    auto& sizes() {
        return m_sizes;
    }

private:
    std::vector<uint8_t> m_sizes;
    std::vector<completion_type> m_completions;
};

struct range {
    uint64_t begin;
    uint64_t end;
    bool is_invalid() const;
    bool is_valid() const;
    bool contains(uint64_t val) const;
};

namespace global {
static const range invalid_range{global::not_found, global::not_found};
}

bool range::is_invalid() const {
    return begin == global::invalid_range.begin or
           end == global::invalid_range.end or begin > end;
}

bool range::is_valid() const {
    return !is_invalid();
}

bool range::contains(uint64_t val) const {
    if (val >= begin and val <= end) return true;
    return false;
}

struct scored_range {
    range r;
    uint32_t min_pos;
    id_type min_val;

    static bool greater(scored_range const& l, scored_range const& r) {
        return l.min_val > r.min_val;
    }
};

template <typename Iterator>
struct scored_range_with_list_iterator {
    typedef Iterator iterator_type;

    scored_range_with_list_iterator()
        : min_pos(global::invalid_term_id)
        , m_open(false) {}

    range r;
    uint32_t min_pos;
    id_type min_val;
    Iterator iterator;

    bool is_open() const {
        return m_open;
    }

    template <typename InvertedIndex>
    void set_iterator(InvertedIndex const& index) {
        assert(min_pos != global::invalid_term_id);
        m_open = true;
        iterator = index.iterator(min_pos);
    }

    id_type minimum() const {
        return is_open() ? *iterator : min_val;
    }

    // static bool greater(scored_range_with_list_iterator const& l,
    //                     scored_range_with_list_iterator const& r) {
    //     return l.minimum() > r.minimum();
    // }

private:
    bool m_open;
};

template <typename Iterator>
struct scored_range_with_list_iterator_comparator {
    bool operator()(scored_range_with_list_iterator<Iterator> const& l,
                    scored_range_with_list_iterator<Iterator> const& r) {
        return l.minimum() > r.minimum();
    }
};

struct byte_range {
    uint8_t const* begin;
    uint8_t const* end;
};

struct uint32_range {
    uint32_t const* begin;
    uint32_t const* end;
};

byte_range string_to_byte_range(std::string const& s) {
    const uint8_t* begin = reinterpret_cast<uint8_t const*>(s.c_str());
    const uint8_t* end = begin + s.size();  // exclude null terminator
    return {begin, end};
}

uint32_range completion_to_uint32_range(completion_type const& c) {
    return {c.data(), c.data() + c.size()};
}

void print(byte_range br) {
    uint32_t size = br.end - br.begin;
    for (uint32_t i = 0; i != size; ++i) {
        std::cout << br.begin[i];
    }
}

void print(uint32_range r) {
    uint32_t size = r.end - r.begin;
    for (uint32_t i = 0; i != size; ++i) {
        std::cout << r.begin[i] << " ";
    }
}

inline int byte_range_compare(byte_range l, byte_range r) {
    int size_l = l.end - l.begin;
    int size_r = r.end - r.begin;
    int n = size_l < size_r ? size_l : size_r;
    int cmp = strncmp(reinterpret_cast<const char*>(l.begin),
                      reinterpret_cast<const char*>(r.begin), n);
    if (cmp != 0) return cmp;
    return size_l - size_r;
}

inline int byte_range_compare(byte_range l, byte_range r, int n) {
    assert(n > 0 and n <= r.end - r.begin);
    int size_l = l.end - l.begin;
    int m = size_l < n ? size_l : n;
    int cmp = strncmp(reinterpret_cast<const char*>(l.begin),
                      reinterpret_cast<const char*>(r.begin), m);
    if (cmp != 0) return cmp;
    return m - n;
}

inline int uint32_range_compare(uint32_range l, uint32_range r) {
    while (l.begin != l.end and r.begin != r.end) {
        uint32_t x = *(l.begin);
        uint32_t y = *(r.begin);
        if (x < y) return -1;
        if (x > y) return 1;
        l.begin += 1;
        r.begin += 1;
    }
    return int(l.end - l.begin) - int(r.end - r.begin);
}

inline int uint32_range_compare(uint32_range l, uint32_range r, uint32_t n) {
    assert(n > 0 and n <= r.end - r.begin);
    uint32_t i = 0;
    while (l.begin != l.end and i != n) {
        uint32_t x = *(l.begin);
        uint32_t y = *(r.begin);
        if (x < y) return -1;
        if (x > y) return 1;
        l.begin += 1;
        r.begin += 1;
        i += 1;
    }
    if (i == n) return 0;
    return -1;
}

void print_completion(completion_type const& c) {
    for (auto x : c) {
        std::cout << x << " ";
    }
}

struct completion_iterator {
    struct value_type {
        id_type doc_id;
        completion_type completion;
    };

    completion_iterator(parameters const& params, std::ifstream& in)
        : m_in(in) {
        m_val.completion.reserve(params.num_levels);
        if (!m_in.good()) {
            throw std::runtime_error(
                "Error in opening file, it may not exist or be malformed.");
        }
        read_next();
    }

    void operator++() {
        read_next();
    }

    value_type& operator*() {
        return m_val;
    }

private:
    value_type m_val;
    std::ifstream& m_in;

    void read_next() {
        m_in >> m_val.doc_id;
        m_val.completion.clear();
        id_type x = global::invalid_term_id;
        while (!m_in.eof() and x != global::terminator) {
            m_in >> x;
            m_val.completion.push_back(x);
        }
    }
};

struct byte_range_iterator {
    byte_range_iterator(const byte_range r)
        : m_r(r) {}

    bool has_next() const {
        return m_r.begin < m_r.end;
    }

    byte_range next() {
        byte_range ret;
        while (m_r.begin != m_r.end and *m_r.begin == ' ') ++m_r.begin;
        ret.begin = m_r.begin;
        while (m_r.begin != m_r.end and *m_r.begin != ' ') ++m_r.begin;
        ret.end = m_r.begin;
        return ret;
    }

private:
    byte_range m_r;
};

typedef std::chrono::high_resolution_clock clock_type;
typedef std::chrono::microseconds duration_type;

template <typename ClockType, typename DurationType>
struct timer {
    timer()
        : m_elapsed(0.0) {}

    void start() {
        m_start = ClockType::now();
    }

    void stop() {
        m_stop = ClockType::now();
        auto e = std::chrono::duration_cast<DurationType>(m_stop - m_start);
        m_elapsed += e.count();
    }

    double elapsed() const {
        return m_elapsed;
    }

private:
    typename ClockType::time_point m_start;
    typename ClockType::time_point m_stop;
    double m_elapsed;
};

typedef timer<clock_type, duration_type> timer_type;

}  // namespace autocomplete
