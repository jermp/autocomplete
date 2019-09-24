#pragma once

#include <vector>
#include <cmath>

#include "util_types.hpp"

namespace autocomplete {

template <uint32_t BucketSize, typename Pointers>
struct fc_dictionary {
    struct builder {
        builder() {}

        builder(parameters const& params) {
            uint32_t n = params.num_terms;
            std::cout << "processing " << n << " strings..." << std::endl;
            uint32_t buckets = std::ceil(double(n) / (BucketSize + 1));
            std::cout << "buckets " << buckets << std::endl;

            m_pointers_to_buckets.reserve(buckets + 1);
            uint32_t tail = n - ((n / (BucketSize + 1)) * (BucketSize + 1)) - 1;
            std::cout << "tail " << tail << std::endl;

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
            other.m_pointers_to_headers.swap(m_pointers_to_headers);
            other.m_pointers_to_buckets.swap(m_pointers_to_buckets);
            other.m_headers.swap(m_headers);
            other.m_buckets.swap(m_buckets);
        }

        void build(fc_dictionary<BucketSize, Pointers>& d) {
            d.m_pointers_to_headers.build(m_pointers_to_headers);
            d.m_pointers_to_buckets.build(m_pointers_to_buckets);
            d.m_headers.swap(m_headers);
            d.m_buckets.swap(m_buckets);
            builder().swap(*this);
        }

    private:
        std::vector<uint32_t> m_pointers_to_headers;
        std::vector<uint32_t> m_pointers_to_buckets;
        std::vector<uint8_t> m_headers;
        std::vector<uint8_t> m_buckets;
    };

    fc_dictionary() {}

    range prefix_range(byte_range t) const {
        // todo
    }

    term_id_type id(byte_range t) const {
        byte_range h;
        term_id_type bucket_id;
        bool is_header = locate_bucket(t, h, bucket_id);
        term_id_type t_id = bucket_id * (BucketSize + 1);
        if (is_header) return t_id;
        return t_id + locate_term(t, h, bucket_id);
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

    size_t bytes() const {
        return m_pointers_to_headers.bytes() + m_pointers_to_buckets.bytes() +
               essentials::vec_bytes(m_headers) +
               essentials::vec_bytes(m_buckets);
    }

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_pointers_to_headers);
        visitor.visit(m_pointers_to_buckets);
        visitor.visit(m_headers);
        visitor.visit(m_buckets);
    }

private:
    Pointers m_pointers_to_headers;
    Pointers m_pointers_to_buckets;
    std::vector<uint8_t> m_headers;
    std::vector<uint8_t> m_buckets;

    bool locate_bucket(byte_range t, byte_range& h,
                       term_id_type& bucket_id) const {
        term_id_type lo = 0;
        term_id_type hi = buckets() - 1;
        term_id_type pos = 0;
        int cmp = 0;

        while (lo <= hi) {
            pos = (lo + hi) / 2;
            h = header(pos);
            cmp = byte_range_compare(h, t);
            if (cmp > 0) {
                hi = pos - 1;
            } else if (cmp < 0) {
                lo = pos + 1;
            } else {
                bucket_id = pos;
                return true;
            }
        }

        if (cmp < 0) {
            bucket_id = pos;
        } else {
            bucket_id = pos - 1;
            h = header(bucket_id);
        }

        return false;
    }

    term_id_type locate_term(byte_range t, byte_range h,
                             term_id_type bucket_id) const {
        assert(bucket_id < buckets());
        static std::string decoded(256 + 1, '\0');
        uint8_t const* begin = h.begin;
        uint8_t i = 0;
        for (; begin[i] != '\0'; ++i) {
            decoded[i] = begin[i];
        }
        decoded[i] = '\0';

        range pointer = m_pointers_to_buckets[bucket_id];
        uint32_t n = pointer.end - pointer.begin;
        uint8_t const* curr = &m_buckets[pointer.begin];
        for (term_id_type i = 1; i <= n; ++i) {
            uint8_t l = *curr;  // |lcp|
            curr += 1;
            for (; *curr != '\0'; ++l, ++curr) {
                decoded[l] = *curr;
            }
            decoded[l] = '\0';
            begin = reinterpret_cast<uint8_t const*>(decoded.c_str());
            int cmp = byte_range_compare(t, {begin, begin + l});
            if (cmp == 0) return i;
            if (cmp < 0) break;  // not found
            curr += 1;
        }

        return global::invalid_term_id;
    }
};

}  // namespace autocomplete