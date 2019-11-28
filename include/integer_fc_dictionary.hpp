#pragma once

#include <cmath>

#include "uint_vec.hpp"
#include "parameters.hpp"
#include "util_types.hpp"
#include "constants.hpp"

namespace autocomplete {

template <uint32_t BucketSize = 16, typename Pointers = uint_vec<uint64_t>>
struct integer_fc_dictionary {
    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_size(params.num_completions) {
            essentials::logger(
                "building integer_fc_dictionary with bucket size " +
                std::to_string(BucketSize) + "...");
            m_docid_to_lexid.resize(params.universe, id_type(-1));

            uint32_t buckets = std::ceil(double(m_size) / (BucketSize + 1));
            m_pointers_to_buckets.reserve(buckets + 1);
            uint32_t tail =
                m_size - ((m_size / (BucketSize + 1)) * (BucketSize + 1));
            if (tail) tail -= 1;  // remove header

            m_pointers_to_headers.push_back(0);
            m_pointers_to_buckets.push_back(0);

            std::ifstream input(
                (params.collection_basename + ".mapped").c_str(),
                std::ios_base::in);
            completion_iterator it(params, input);

            id_type lex_id = 0;
            for (uint32_t b = 0; b != buckets; ++b) {
                auto& header = *it;
                m_docid_to_lexid[header.doc_id] = lex_id++;
                write_header(header.completion);
                m_pointers_to_headers.push_back(m_headers.size());
                completion_type prev;
                prev.swap(header.completion);
                ++it;
                uint32_t size = b != buckets - 1 ? BucketSize : tail;
                for (uint32_t i = 0; i != size; ++i, ++it) {
                    auto& record = *it;
                    auto& curr = record.completion;
                    m_docid_to_lexid[record.doc_id] = lex_id++;
                    uint32_t l = 0;  // |lcp(curr,prev)|
                    while (l != curr.size() and l != prev.size() and
                           curr[l] == prev[l]) {
                        ++l;
                    }
                    write(curr, l);
                    prev.swap(curr);
                }
                m_pointers_to_buckets.push_back(m_buckets.size());
            }

            // NOTE: pad to allow fixed-copy operations
            for (uint32_t i = 0;
                 i != constants::MAX_NUM_TERMS_PER_QUERY * sizeof(uint32_t);
                 ++i) {
                m_buckets.push_back(0);
            }

            input.close();
            essentials::logger("DONE");
        }

        void swap(builder& other) {
            std::swap(other.m_size, m_size);
            other.m_pointers_to_headers.swap(m_pointers_to_headers);
            other.m_pointers_to_buckets.swap(m_pointers_to_buckets);
            other.m_headers.swap(m_headers);
            other.m_buckets.swap(m_buckets);
            other.m_docid_to_lexid.swap(m_docid_to_lexid);
        }

        void build(integer_fc_dictionary<BucketSize, Pointers>& d) {
            d.m_size = m_size;
            d.m_pointers_to_headers.build(m_pointers_to_headers);
            d.m_pointers_to_buckets.build(m_pointers_to_buckets);
            d.m_headers.swap(m_headers);
            d.m_buckets.swap(m_buckets);
            builder().swap(*this);
        }

        std::vector<id_type>& docid_to_lexid() {
            return m_docid_to_lexid;
        }

    private:
        size_t m_size;
        std::vector<uint64_t> m_pointers_to_headers;
        std::vector<uint64_t> m_pointers_to_buckets;
        std::vector<uint32_t> m_headers;
        std::vector<uint8_t> m_buckets;
        std::vector<id_type> m_docid_to_lexid;

        void write_header(completion_type const& c) {
            assert(c.size() > 0 and
                   c.size() <= constants::MAX_NUM_TERMS_PER_QUERY);
            assert(c.back() == global::terminator);
            m_headers.insert(m_headers.end(), c.begin(),
                             c.begin() + c.size() - 1);
        }

        void write(completion_type const& c, uint32_t lcp) {
            assert(c.size() > 0 and
                   c.size() <= constants::MAX_NUM_TERMS_PER_QUERY);
            assert(c.back() == global::terminator);
            m_buckets.push_back(lcp);
            uint8_t size = c.size() - 1;  // discard terminator
            assert(size >= lcp);
            m_buckets.push_back(size - lcp);
            uint8_t const* begin = reinterpret_cast<uint8_t const*>(c.data());
            m_buckets.insert(m_buckets.end(), begin + lcp * sizeof(id_type),
                             begin + size * sizeof(c.front()));
        }
    };

    integer_fc_dictionary() {}

    // NOTE: return 0-based ids
    // NOTE: not used
    id_type locate(uint32_range c) const {
        uint32_range h;
        id_type bucket_id;
        bool is_header = locate_bucket(c, h, bucket_id);
        id_type base_id = bucket_id * (BucketSize + 1);
        if (is_header) return base_id;
        id_type offset_id = locate(c, h, bucket_id);
        if (offset_id == global::invalid_term_id) {
            return global::invalid_term_id;
        }
        return base_id + offset_id;
    }

    // If the last token of the query is not completely specified,
    // then we search for its lexicographic range among the children of c.
    // Return [a,b)
    range locate_prefix(completion_type& prefix, range suffix_lex_range) const {
        prefix.push_back(suffix_lex_range.begin);
        uint32_range h_begin;
        id_type bucket_id_begin;
        bool is_header_begin = locate_bucket(completion_to_uint32_range(prefix),
                                             h_begin, bucket_id_begin);
        uint32_t p_begin = bucket_id_begin * (BucketSize + 1);
        if (!is_header_begin) {
            p_begin += left_locate(completion_to_uint32_range(prefix), h_begin,
                                   bucket_id_begin);
        }

        prefix.pop_back();

        uint32_range h_end;
        id_type bucket_id_end;

        prefix.push_back(suffix_lex_range.end);

        if (suffix_lex_range.begin ==
            suffix_lex_range.end) {  // trick to force a right search
            prefix.push_back(global::invalid_term_id);
        }

        locate_right_bucket(completion_to_uint32_range(prefix), h_end,
                            bucket_id_end,
                            bucket_id_begin  // hint
        );
        uint32_t p_end = bucket_id_end * (BucketSize + 1);
        p_end += right_locate(completion_to_uint32_range(prefix), h_end,
                              bucket_id_end);

        prefix.pop_back();

        if (p_end < p_begin) {
            return global::invalid_range;
        }

        if (suffix_lex_range.begin == suffix_lex_range.end) {
            prefix.pop_back();
        }

        return {p_begin, p_end + 1};
    }

    // NOTE: 0-based ids
    // returns the length of the extracted string (in num of terms)
    uint8_t extract(id_type id, completion_type& c) const {
        uint32_t bucket_id = id / (BucketSize + 1);
        uint32_t k = id % (BucketSize + 1);
        return extract(k, bucket_id, c);
    }

    size_t size() const {
        return m_size;
    }

    size_t bucket_size(uint32_t bucket_id) const {
        if (bucket_id != buckets() - 1) return BucketSize;
        size_t tail = size() - ((size() / (BucketSize + 1)) * (BucketSize + 1));
        if (tail) tail -= 1;
        return tail;
    }

    size_t buckets() const {
        return m_pointers_to_headers.size() - 1;
    }

    uint32_range header(uint32_t i) const {
        assert(i < buckets());
        range pointer = m_pointers_to_headers[i];
        return {m_headers.data() + pointer.begin,
                m_headers.data() + pointer.end};
    }

    size_t data_bytes() const {
        return essentials::vec_bytes(m_headers) +
               essentials::vec_bytes(m_buckets);
    }

    size_t pointer_bytes() const {
        return m_pointers_to_headers.bytes() + m_pointers_to_buckets.bytes();
    }

    size_t bytes() const {
        return essentials::pod_bytes(m_size) + m_pointers_to_headers.bytes() +
               m_pointers_to_buckets.bytes() +
               essentials::vec_bytes(m_headers) +
               essentials::vec_bytes(m_buckets);
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_size);
        visitor.visit(m_pointers_to_headers);
        visitor.visit(m_pointers_to_buckets);
        visitor.visit(m_headers);
        visitor.visit(m_buckets);
    }

private:
    size_t m_size;
    Pointers m_pointers_to_headers;
    Pointers m_pointers_to_buckets;
    std::vector<uint32_t> m_headers;
    std::vector<uint8_t> m_buckets;

    bool locate_bucket(uint32_range t, uint32_range& h, id_type& bucket_id,
                       int lower_bound_hint = 0) const {
        int lo = lower_bound_hint, hi = buckets() - 1, mi = 0, cmp = 0;

        while (lo <= hi) {
            mi = (lo + hi) / 2;
            h = header(mi);
            cmp = uint32_range_compare(h, t);
            if (cmp > 0) {
                hi = mi - 1;
            } else if (cmp < 0) {
                lo = mi + 1;
            } else {
                bucket_id = mi;
                return true;
            }
        }

        if (cmp < 0) {
            bucket_id = mi;
        } else {
            bucket_id = hi == -1 ? 0 : hi;
            h = header(bucket_id);
        }

        return false;
    }

    void locate_right_bucket(uint32_range t, uint32_range& h,
                             id_type& bucket_id,
                             int lower_bound_hint = 0) const {
        int lo = lower_bound_hint, hi = buckets() - 1, mi = 0, cmp = 0;
        size_t n = t.end - t.begin;
        while (lo <= hi) {
            mi = (lo + hi) / 2;
            h = header(mi);
            cmp = uint32_range_compare(h, t, n);
            if (cmp > 0) {
                hi = mi - 1;
            } else if (cmp <= 0) {
                lo = mi + 1;
            }
        }

        if (cmp < 0) {
            bucket_id = mi;
        } else {
            bucket_id = hi == -1 ? 0 : hi;
            h = header(bucket_id);
        }
    }

#define INT_FC_DICT_LOCATE_INIT                                      \
    static uint32_t decoded[2 * constants::MAX_NUM_TERMS_PER_QUERY]; \
    memcpy(decoded, h.begin, (h.end - h.begin) * sizeof(uint32_t));  \
    uint8_t lcp_len;                                                 \
    uint32_t n = bucket_size(bucket_id);                             \
    uint8_t const* curr =                                            \
        m_buckets.data() + m_pointers_to_buckets.access(bucket_id);

    // return the length of the decoded string
    uint8_t decode(uint8_t const* in, uint32_t* out, uint8_t* lcp_len) const {
        *lcp_len = *in++;  // |lcp|
        uint8_t l = *lcp_len;
        uint8_t suffix_len = *in++;
        memcpy(out + l, reinterpret_cast<uint32_t const*>(in),
               constants::MAX_NUM_TERMS_PER_QUERY * sizeof(uint32_t));
        return l + suffix_len;
    }

    uint8_t extract(id_type id, id_type bucket_id, completion_type& c) const {
        auto h = header(bucket_id);
        memcpy(c.data(), h.begin, (h.end - h.begin) * sizeof(uint32_t));
        uint8_t lcp_len;
        assert(id <= bucket_size(bucket_id));
        uint8_t const* curr =
            m_buckets.data() + m_pointers_to_buckets.access(bucket_id);
        uint8_t string_len = h.end - h.begin;
        for (id_type i = 1; i <= id; ++i) {
            string_len = decode(curr, c.data(), &lcp_len);
            curr += (string_len - lcp_len) * sizeof(uint32_t) + 2;
        }
        return string_len;
    }

    id_type locate(uint32_range t, uint32_range h, id_type bucket_id) const {
        INT_FC_DICT_LOCATE_INIT
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = uint32_range_compare(t, {decoded, decoded + l});
            if (cmp == 0) return i;
            if (cmp < 0) return global::invalid_term_id;
            curr += (l - lcp_len) * sizeof(uint32_t) + 2;
        }
        assert(false);
        __builtin_unreachable();
    }

    id_type right_locate(uint32_range p, uint32_range h,
                         id_type bucket_id) const {
        INT_FC_DICT_LOCATE_INIT
        uint32_t len = p.end - p.begin;
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = uint32_range_compare({decoded, decoded + l}, p, len);
            if (cmp > 0) return i - 1;
            curr += (l - lcp_len) * sizeof(uint32_t) + 2;
        }
        return n;
    }

    id_type left_locate(uint32_range p, uint32_range h,
                        id_type bucket_id) const {
        INT_FC_DICT_LOCATE_INIT
        uint32_t len = p.end - p.begin;
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = uint32_range_compare({decoded, decoded + l}, p, len);
            if (cmp >= 0) return i;
            curr += (l - lcp_len) * sizeof(uint32_t) + 2;
        }
        return n + 1;
    }
};

}  // namespace autocomplete