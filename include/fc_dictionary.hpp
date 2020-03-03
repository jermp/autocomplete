#pragma once

#include <cmath>

#include "uint_vec.hpp"
#include "parameters.hpp"
#include "util_types.hpp"
#include "constants.hpp"

namespace autocomplete {

template <uint32_t BucketSize = 16, typename Pointers = uint_vec<uint32_t>>
struct fc_dictionary {
    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_size(params.num_terms) {
            essentials::logger("building fc_dictionary with bucket size " +
                               std::to_string(BucketSize) + "...");

            uint32_t buckets = std::ceil(double(m_size) / (BucketSize + 1));
            m_pointers_to_buckets.reserve(buckets + 1);
            uint32_t tail =
                m_size - ((m_size / (BucketSize + 1)) * (BucketSize + 1));
            if (tail) tail -= 1;  // remove header

            m_pointers_to_headers.push_back(0);
            m_pointers_to_buckets.push_back(0);

            std::ifstream input((params.collection_basename + ".dict").c_str(),
                                std::ios_base::in);
            if (!input.good()) {
                throw std::runtime_error("Dictionary file not found");
            }
            std::string prev = "";
            std::string curr;
            std::string header;

            uint64_t total_characters = 0;
            for (uint32_t b = 0; b != buckets; ++b) {
                input >> header;
                total_characters += header.size();
                write_header(header);
                m_pointers_to_headers.push_back(m_headers.size());
                prev.swap(header);
                uint32_t size = b != buckets - 1 ? BucketSize : tail;
                for (uint32_t i = 0; i != size; ++i) {
                    input >> curr;
                    total_characters += curr.size();
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
            for (uint32_t i = 0; i != constants::MAX_NUM_CHARS_PER_QUERY; ++i) {
                m_buckets.push_back(0);
            }

            std::cout << static_cast<double>(total_characters) / m_size
                      << " characters per string" << std::endl;

            input.close();
            essentials::logger("DONE");
        }

        void swap(builder& other) {
            std::swap(other.m_size, m_size);
            other.m_pointers_to_headers.swap(m_pointers_to_headers);
            other.m_pointers_to_buckets.swap(m_pointers_to_buckets);
            other.m_headers.swap(m_headers);
            other.m_buckets.swap(m_buckets);
        }

        void build(fc_dictionary<BucketSize, Pointers>& d) {
            d.m_size = m_size;
            d.m_pointers_to_headers.build(m_pointers_to_headers);
            d.m_pointers_to_buckets.build(m_pointers_to_buckets);
            d.m_headers.swap(m_headers);
            d.m_buckets.swap(m_buckets);
            builder().swap(*this);
        }

    private:
        size_t m_size;
        std::vector<uint32_t> m_pointers_to_headers;
        std::vector<uint32_t> m_pointers_to_buckets;
        std::vector<uint8_t> m_headers;
        std::vector<uint8_t> m_buckets;

        void write_header(std::string const& header) {
            assert(header.size() > 0 and
                   header.size() <= constants::MAX_NUM_CHARS_PER_QUERY);
            m_headers.insert(m_headers.end(), header.begin(), header.end());
        }

        void write(std::string const& s, uint32_t lcp) {
            assert(s.size() > 0 and
                   s.size() <= constants::MAX_NUM_CHARS_PER_QUERY);
            m_buckets.push_back(lcp);
            uint8_t size = s.size();
            assert(size >= lcp);
            m_buckets.push_back(size - lcp);
            m_buckets.insert(m_buckets.end(), s.begin() + lcp, s.end());
        }
    };

    fc_dictionary() {}

    // NOTE: return inclusive ranges, i.e., [a,b]
    // 0-based ids
    range locate_prefix(byte_range p) const {
        if (p.end - p.begin == 0) return {0, size() - 1};
        auto bucket_id = locate_buckets(p);
        byte_range h_begin = header(bucket_id.begin);
        byte_range h_end = header(bucket_id.end);
        uint32_t p_begin = bucket_id.begin * (BucketSize + 1);
        uint32_t p_end = bucket_id.end * (BucketSize + 1);
        if (byte_range_compare(h_begin, p) != 0) {
            p_begin += left_locate(p, h_begin, bucket_id.begin);
        }
        p_end += right_locate(p, h_end, bucket_id.end);
        if (p_end < p_begin) return global::invalid_range;
        return {p_begin, p_end};
    }

    // NOTE: the dictionary returns
    // 1-based ids because the id 0 is reserved
    // to mark the end of a string
    id_type locate(byte_range t) const {
        byte_range h;
        id_type bucket_id;
        bool is_header = locate_bucket(t, h, bucket_id);
        id_type base_id = bucket_id * (BucketSize + 1) + 1;
        if (is_header) return base_id;
        id_type offset_id = locate(t, h, bucket_id);
        if (offset_id == global::invalid_term_id) {
            return global::invalid_term_id;
        }
        return base_id + offset_id;
    }

    // return the length of the extracted string
    uint8_t extract(id_type id, uint8_t* out) const {
        assert(id > 0);
        id -= 1;
        uint32_t bucket_id = id / (BucketSize + 1);
        uint32_t k = id % (BucketSize + 1);
        return extract(k, bucket_id, out);
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

    byte_range header(uint32_t i) const {
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
    std::vector<uint8_t> m_headers;
    std::vector<uint8_t> m_buckets;

    bool locate_bucket(byte_range t, byte_range& h, id_type& bucket_id) const {
        int lo = 0, hi = buckets() - 1, mi = 0, cmp = 0;

        while (lo <= hi) {
            mi = (lo + hi) / 2;
            h = header(mi);
            cmp = byte_range_compare(h, t);
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

    range locate_buckets(byte_range p) const {
        range r;
        uint32_t n = p.end - p.begin;
        int lo, hi, left, right;

        // 1. locate left bucket
        lo = 0;
        hi = buckets() - 1;
        while (lo <= hi) {
            int mi = (lo + hi) / 2;
            byte_range h = header(mi);
            int cmp = byte_range_compare(h, p, n);
            if (cmp >= 0) {
                hi = mi - 1;
            } else {
                lo = mi + 1;
            }
        }

        if (uint32_t(lo) == buckets()) {
            r.begin = lo - 1;
            r.end = lo - 1;
            return r;
        }

        if (lo == 0) {
            left = 0;
        } else {
            left = byte_range_compare(header(lo), p) == 0 ? lo : lo - 1;
        }

        // 2. if the left + 1 bucket's header has a prefix of size n that is
        //    larger than p, then all strings prefixed by p are in the same
        //    bucket (or if we are in the last bucket)
        if (uint32_t(left) == buckets() - 1 or
            byte_range_compare(header(left + 1), p, n) > 0) {
            r.begin = left;
            r.end = left;
            return r;
        }

        // 3. otherwise, locate the right bucket
        lo = left;
        hi = buckets() - 1;
        while (lo <= hi) {
            int mi = (lo + hi) / 2;
            byte_range h = header(mi);
            int cmp = byte_range_compare(h, p, n);
            if (cmp <= 0) {
                lo = mi + 1;
            } else {
                hi = mi - 1;
            }
        }

        right = hi;

        r.begin = left;
        r.end = right;
        return r;
    }

#define FC_DICT_LOCATE_INIT                                         \
    static uint8_t decoded[2 * constants::MAX_NUM_CHARS_PER_QUERY]; \
    memcpy(decoded, h.begin, h.end - h.begin);                      \
    uint8_t lcp_len;                                                \
    uint32_t n = bucket_size(bucket_id);                            \
    uint8_t const* curr =                                           \
        m_buckets.data() + m_pointers_to_buckets.access(bucket_id);

    uint8_t decode(uint8_t const* in, uint8_t* out, uint8_t* lcp_len) const {
        *lcp_len = *in++;  // |lcp|
        uint8_t l = *lcp_len;
        uint8_t suffix_len = *in++;

        // NOTE 1: excluding null terminators, allow us to use memcpy here
        // because we know exactly how many bytes to copy: this is much faster
        // than looping until we hit '\0'.

        // NOTE 2: always copying a fixed amount
        // of bytes (constants::MAX_NUM_CHARS_PER_QUERY) is much faster than
        // copying an exact amount, e.g., suffix_len (althoung it could be
        // less), so do not do: memcpy(out+ l, in, suffix_len).

        memcpy(out + l, in, constants::MAX_NUM_CHARS_PER_QUERY);

        return l + suffix_len;
    }

    uint8_t extract(id_type id, id_type bucket_id, uint8_t* out) const {
        assert(id <= bucket_size(bucket_id));
        byte_range h = header(bucket_id);
        memcpy(out, h.begin, h.end - h.begin);
        uint8_t lcp_len;
        uint8_t const* curr =
            m_buckets.data() + m_pointers_to_buckets.access(bucket_id);
        uint8_t string_len = h.end - h.begin;
        for (id_type i = 1; i <= id; ++i) {
            string_len = decode(curr, out, &lcp_len);
            curr += string_len - lcp_len + 2;
        }
        return string_len;
    }

    id_type locate(byte_range t, byte_range h, id_type bucket_id) const {
        FC_DICT_LOCATE_INIT
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = byte_range_compare(t, {decoded, decoded + l});
            if (cmp == 0) return i;
            if (cmp < 0) return global::invalid_term_id;
            curr += l - lcp_len + 2;
        }
        return global::invalid_term_id;  // term does not exist in dictionary
    }

    id_type left_locate(byte_range p, byte_range h, id_type bucket_id) const {
        FC_DICT_LOCATE_INIT
        uint32_t len = p.end - p.begin;
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = byte_range_compare({decoded, decoded + l}, p, len);
            if (cmp == 0) return i;
            curr += l - lcp_len + 2;
        }
        return n + 1;
    }

    id_type right_locate(byte_range p, byte_range h, id_type bucket_id) const {
        FC_DICT_LOCATE_INIT
        uint32_t len = p.end - p.begin;
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = byte_range_compare({decoded, decoded + l}, p, len);
            if (cmp > 0) return i - 1;
            curr += l - lcp_len + 2;
        }
        return n;
    }
};

}  // namespace autocomplete