#pragma once

#include <vector>
#include <functional>

#include "util.hpp"

namespace autocomplete {

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
    std::cout << std::endl;
}

int byte_range_compare(byte_range l, byte_range r) {
    while (l.begin != l.end and r.begin != r.end and *(l.begin) == *(r.begin)) {
        ++l.begin;
        ++r.begin;
    }
    return *(l.begin) - *(r.begin);
}

// compare l with the prefix of r of size n
int byte_range_compare(byte_range l, byte_range r, uint32_t n) {
    assert(n <= r.end - r.begin);
    uint32_t i = 0;
    if (n > 1) {
        while (l.begin != l.end and i != n - 1 and *(l.begin) == *(r.begin)) {
            ++i;
            ++l.begin;
            ++r.begin;
        }
    }
    return *(l.begin) - *(r.begin);
}

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

uint32_t parse(std::string& s) {
    uint32_t num_terms = 1;
    for (uint64_t i = 0; i != s.size(); ++i) {
        if (s[i] == ' ') {
            s[i] = '\0';
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

}  // namespace autocomplete
