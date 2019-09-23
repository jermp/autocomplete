#pragma once

#include <vector>
#include <fstream>

namespace autocomplete {

// assume 32 bits are enough to store
// both a term_id and a doc_id
typedef uint32_t term_id_type;
typedef uint32_t doc_id_type;

namespace global {
static const doc_id_type invalid_doc_id = doc_id_type(-1);
static const term_id_type invalid_term_id = term_id_type(-1);
static const term_id_type terminator = 0;
}  // namespace global

struct range {
    uint64_t begin, end;
};

struct parameters {
    parameters()
        : num_completions(0)
        , num_levels(0)
        , collection_basename(nullptr) {}

    void load() {
        std::string filename = std::string(collection_basename) + ".stats";
        std::ifstream input(filename.c_str(), std::ios_base::in);
        if (!input.good()) {
            throw std::runtime_error("File with statistics not found");
        }
        input >> num_completions;
        input >> num_levels;
        assert(num_completions > 0);
        assert(num_levels > 0);
        nodes_per_level.resize(num_levels, 0);
        for (uint32_t i = 0; i != num_levels; ++i) {
            input >> nodes_per_level[i];
        }
        // for (auto x : nodes_per_level) {
        //     std::cout << x << std::endl;
        // }
    }

    uint32_t num_completions;
    uint32_t num_levels;
    std::vector<uint32_t> nodes_per_level;
    char const* collection_basename;
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

    doc_id_type doc_id;
    std::vector<term_id_type> term_ids;
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
