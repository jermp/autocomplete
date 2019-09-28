#pragma once

namespace autocomplete {

typedef std::vector<id_type> completion_type;

struct completion_comparator {
    bool operator()(completion_type const& lhs,
                    completion_type const& rhs) const {
        size_t l = 0;  // |lcp(lhs,rhs)|
        while (l < lhs.size() - 1 and l < rhs.size() - 1 and lhs[l] == rhs[l]) {
            ++l;
        }
        return lhs[l] < rhs[l];
    }
};

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

}  // namespace autocomplete