#pragma once

namespace autocomplete {

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

    void push_back(id_type t) {
        term_ids.push_back(t);
    }

    id_type doc_id;
    std::vector<id_type> term_ids;
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
        id_type x = global::invalid_term_id;
        while (!m_in.eof() and x != global::terminator) {
            m_in >> x;
            m_val.term_ids.push_back(x);
        }
    }
};

}  // namespace autocomplete