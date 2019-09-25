#pragma once

#include <vector>
#include <fstream>

#include "util.hpp"

namespace autocomplete {

struct range {
    uint64_t begin;
    uint64_t end;
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

struct parameters {
    parameters()
        : num_terms(0)
        , num_completions(0)
        , num_levels(0) {}

    void load() {
        std::ifstream input((collection_basename + ".mapped.stats").c_str(),
                            std::ios_base::in);
        if (!input.good()) {
            throw std::runtime_error("File with statistics not found");
        }
        input >> num_terms;
        input >> num_completions;
        input >> num_levels;
        assert(num_terms > 0);
        assert(num_completions > 0);
        assert(num_levels > 0);
        nodes_per_level.resize(num_levels, 0);
        for (uint32_t i = 0; i != num_levels; ++i) {
            input >> nodes_per_level[i];
        }
    }

    uint32_t num_terms;
    uint32_t num_completions;
    uint32_t num_levels;
    std::vector<uint32_t> nodes_per_level;
    std::string collection_basename;
};

struct completion {
    completion(uint32_t size)  // in terms
        : doc_id(global::invalid_doc_id) {
        term_ids.reserve(size);
    }

    static completion empty() {
        completion val(1);
        val.term_ids.push_back(global::terminator);
        return val;
    }

    friend std::ostream& operator<<(std::ostream& os, completion const& rhs) {
        os << "(";
        for (size_t i = 0; i != rhs.term_ids.size(); ++i) {
            os << rhs.term_ids[i];
            if (i != rhs.term_ids.size() - 1) os << ",";
        }
        os << ")";
        return os;
    }

    size_t size() const {
        assert(term_ids.size() > 0);
        return term_ids.size() - 1;
    }

    uint32_t operator[](size_t i) const {
        assert(i < term_ids.size());
        return term_ids[i];
    }

    void swap(completion& other) {
        term_ids.swap(other.term_ids);
    }

    void push_back(term_id_type t) {
        term_ids.push_back(t);
    }

    doc_id_type doc_id;
    std::vector<term_id_type> term_ids;
};

struct completion_comparator {
    bool operator()(completion const& lhs, completion const& rhs) const {
        size_t l = 0;  // |lcp(lhs,rhs)|
        while (l < lhs.size() and l < rhs.size() and lhs[l] == rhs[l]) {
            ++l;
        }
        return lhs[l] < rhs[l];
    }
};

struct completion_iterator {
    completion_iterator(parameters const& params, std::ifstream& in)
        : m_val(params.num_levels)
        , m_in(in) {
        if (!m_in.good()) {
            throw std::runtime_error(
                "Error in opening file, it may not exist or be malformed.");
        }
        read_next();
    }

    void operator++() {
        read_next();
    }

    completion& operator*() {
        return m_val;
    }

private:
    completion m_val;
    std::ifstream& m_in;

    void read_next() {
        m_in >> m_val.doc_id;
        m_val.term_ids.clear();
        term_id_type x = global::invalid_term_id;
        while (!m_in.eof() and x != global::terminator) {
            m_in >> x;
            m_val.term_ids.push_back(x);
        }
    }
};

}  // namespace autocomplete
