#pragma once

#include <vector>
#include <functional>
#include <chrono>

#include "util.hpp"

namespace autocomplete {

typedef std::vector<id_type> completion_type;

struct range {
    uint64_t begin;
    uint64_t end;

    // friend std::ostream& operator<<(std::ostream& os, range const& rhs) {
    //     os << "[" << rhs.begin << "," << rhs.end << "]";
    //     return os;
    // }
};

struct scored_range {
    range r;
    uint32_t min_pos;
    id_type min_val;
};

typedef std::function<bool(scored_range const&, scored_range const&)>
    scored_range_comparator_type;
scored_range_comparator_type scored_range_comparator =
    [](scored_range const& l, scored_range const& r) {
        return l.min_val > r.min_val;
    };

struct byte_range {
    uint8_t const* begin;
    uint8_t const* end;
};

struct scored_byte_range {
    byte_range string;
    id_type score;
};

byte_range string_to_byte_range(std::string const& s) {
    const uint8_t* begin = reinterpret_cast<uint8_t const*>(s.c_str());
    const uint8_t* end = begin + s.size() + 1;  // for '\0' terminator
    return {begin, end};
}

void print(byte_range br) {
    uint32_t size = br.end - br.begin;
    for (uint32_t i = 0; i != size; ++i) {
        std::cout << br.begin[i];
    }
}

inline int byte_range_compare(byte_range l, byte_range r) {
    return strcmp(reinterpret_cast<const char*>(l.begin),
                  reinterpret_cast<const char*>(r.begin));
}

inline int byte_range_compare(byte_range l, byte_range r, uint32_t n) {
    return strncmp(reinterpret_cast<const char*>(l.begin),
                   reinterpret_cast<const char*>(r.begin), n);
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

// NOTE: this has log complexity but if k is small...
struct topk_queue {
    void push(scored_range sr) {
        m_q.push_back(sr);
        std::push_heap(m_q.begin(), m_q.end(), scored_range_comparator);
    }

    scored_range top() {
        return m_q.front();
    }

    void pop() {
        std::pop_heap(m_q.begin(), m_q.end(), scored_range_comparator);
        m_q.pop_back();
    }

    void clear() {
        m_q.clear();
    }

private:
    std::vector<scored_range> m_q;
};

uint32_t parse_query(std::string& query) {
    uint32_t num_terms = 1;
    for (uint64_t i = 0; i != query.size(); ++i) {
        if (query[i] == ' ') {
            query[i] = '\0';
            ++num_terms;
        }
    }
    return num_terms;
}

struct forward_byte_range_iterator {
    forward_byte_range_iterator(byte_range const& r) {
        init(r);
    }

    void init(byte_range const& r, char separator = '\0') {
        m_cur_pos = r.begin;
        m_begin = r.begin;
        m_end = r.end;
        m_separator = separator;
    }

    byte_range next() {
        uint8_t const* pos = m_cur_pos;
        for (; pos != m_end; ++pos) {
            if (*pos == m_separator or *pos == '\n') break;
        }
        byte_range br = {m_cur_pos, pos};
        m_cur_pos = pos + 1;
        return br;
    }

private:
    uint8_t const* m_cur_pos;
    uint8_t const* m_begin;
    uint8_t const* m_end;
    char m_separator;
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
