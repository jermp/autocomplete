#pragma once

#include <cmath>

#include "parameters.hpp"
#include "util_types.hpp"

namespace autocomplete {

template <uint32_t BucketSize, typename Pointers>
struct fc_dictionary {
    struct builder {
        builder() {}

        builder(parameters const& params)
            : m_size(params.num_terms) {
            // std::cout << "processing " << m_size << " strings..." <<
            // std::endl;
            uint32_t buckets = std::ceil(double(m_size) / (BucketSize + 1));
            // std::cout << "buckets " << buckets << std::endl;

            m_pointers_to_buckets.reserve(buckets + 1);
            uint32_t tail =
                m_size - ((m_size / (BucketSize + 1)) * (BucketSize + 1)) - 1;
            // std::cout << "tail " << tail << std::endl;

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

            for (uint32_t b = 0; b != buckets; ++b) {
                input >> header;
                m_headers.insert(m_headers.end(), header.begin(), header.end());
                m_headers.push_back('\0');
                m_pointers_to_headers.push_back(m_headers.size());
                prev.swap(header);
                uint32_t size = b != buckets - 1 ? BucketSize : tail;
                for (uint32_t i = 0; i != size; ++i) {
                    input >> curr;
                    uint32_t l = 0;  // |lcp(curr,prev)|
                    while (l != curr.size() and l != prev.size() and
                           curr[l] == prev[l]) {
                        ++l;
                    }
                    assert(l < 256);  // 1 byte is enough
                    m_buckets.push_back(l);
                    m_buckets.insert(m_buckets.end(), curr.begin() + l,
                                     curr.end());
                    m_buckets.push_back('\0');
                    prev.swap(curr);
                }
                m_pointers_to_buckets.push_back(m_buckets.size());
            }
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
    };

    fc_dictionary() {}

    range locate_prefix(byte_range p) const {
        auto bucket_id = locate_buckets(p);
        byte_range h_begin = header(bucket_id.begin);
        byte_range h_end = header(bucket_id.end);
        uint32_t p_begin = bucket_id.begin * (BucketSize + 1);
        uint32_t p_end = bucket_id.end * (BucketSize + 1);
        if (byte_range_compare(h_begin, p) != 0) {
            p_begin += left_locate(p, h_begin, bucket_id.begin);
        }
        p_end += right_locate(p, h_end, bucket_id.end);
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
        uint32_t n = size();
        return bucket_id != buckets() - 1
                   ? BucketSize
                   : (n - ((n / (BucketSize + 1)) * (BucketSize + 1)) - 1);
    }

    size_t buckets() const {
        return m_pointers_to_headers.size() - 1;
    }

    byte_range header(uint32_t i) const {
        assert(i < buckets());
        range pointer = m_pointers_to_headers[i];
        assert(m_headers[pointer.end - 1] == '\0');
        return {m_headers.data() + pointer.begin,
                m_headers.data() + pointer.end - 1};
    }

    // void print_stats(essentials::json_lines& stats, size_t bytes);

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
        int lo = 0, hi = buckets() - 1;
        int mi, cmp;

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
            bucket_id = mi - 1;
            h = header(bucket_id);
        }

        return false;
    }

    range locate_buckets(byte_range p) const {
        range r;
        uint32_t n = p.end - p.begin - 1;
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

#define LOCATE_INIT                                 \
    static uint8_t* decoded = new uint8_t[256 + 1]; \
    memcpy(decoded, h.begin, h.end - h.begin + 1);  \
    uint8_t lcp_len;                                \
    uint32_t n = bucket_size(bucket_id);            \
    uint8_t const* curr =                           \
        m_buckets.data() + m_pointers_to_buckets[bucket_id].begin;

    uint8_t decode(uint8_t const* in, uint8_t* out, uint8_t* lcp_len) const {
        *lcp_len = *in++;  // |lcp|
        uint8_t l = *lcp_len;
        while (*in) out[l++] = *in++;
        out[l] = '\0';
        return l;
    }

    uint8_t extract(id_type id, id_type bucket_id, uint8_t* out) const {
        byte_range h = header(bucket_id);
        memcpy(out, h.begin, h.end - h.begin + 1);
        uint8_t lcp_len;
        assert(id <= bucket_size(bucket_id));
        uint8_t const* curr =
            m_buckets.data() + m_pointers_to_buckets[bucket_id].begin;
        uint8_t string_len = h.end - h.begin;
        for (id_type i = 1; i <= id; ++i) {
            string_len = decode(curr, out, &lcp_len);
            curr += string_len - lcp_len + 2;
        }
        return string_len;
    }

    id_type locate(byte_range t, byte_range h, id_type bucket_id) const {
        LOCATE_INIT
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = byte_range_compare(t, {decoded, decoded + l});
            if (cmp == 0) return i;
            if (cmp < 0) return global::invalid_term_id;
            curr += l - lcp_len + 2;
        }
        assert(false);
        __builtin_unreachable();
    }

    id_type left_locate(byte_range p, byte_range h, id_type bucket_id) const {
        LOCATE_INIT
        uint32_t len = p.end - p.begin - 1;
        for (id_type i = 1; i <= n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = byte_range_compare({decoded, decoded + l}, p, len);
            if (cmp == 0) return i;
            curr += l - lcp_len + 2;
        }
        return n + 1;
    }

    id_type right_locate(byte_range p, byte_range h, id_type bucket_id) const {
        LOCATE_INIT
        uint32_t len = p.end - p.begin - 1;
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