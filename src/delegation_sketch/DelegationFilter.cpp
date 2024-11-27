#include "DelegationFilter.hpp"
#include <algorithm>

// DelegationFilter implementation
DelegationFilter::DelegationFilter() {}

DelegationFilter::DelegationFilter(int max_size) {
    keys = std::vector<int>(max_size);
    counts = std::vector<int>(max_size);
    FILTER_SIZE = max_size;
    size = 0;
    lock = false;
}

DelegationFilter::DelegationFilter(DelegationFilter &&other) {
    keys = std::move(other.keys);
    counts = std::move(other.counts);
    size.store(other.size.load(std::memory_order_relaxed), std::memory_order_relaxed);
    lock.store(other.lock.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

int DelegationFilter::lookup_value(const int &key) {
    auto it = std::find(keys.begin(), keys.end(), key);
    if (it != keys.end()) { return counts[it - keys.begin()]; }
    return 0;
}

int DelegationFilter::lookup_index(const int &key) {
    auto it = std::find(keys.begin(), keys.end(), key);
    if (it != keys.end()) { return it - keys.begin(); }
    return -1;
}

int DelegationFilter::update_or_insert_if_not_full(const int &key) {
    auto it = std::find(keys.begin(), keys.end(), key);
    if (it != keys.end()) {
        counts[it - keys.begin()] += 1;
        return 1;
    }

    int size = this->size.load(std::memory_order_relaxed);

    if (size < FILTER_SIZE) {
        keys[size] = key;
        counts[size] = 1;
        this->size.store(size + 1, std::memory_order_relaxed);
        return 1;
    }
    return 0;
}

int DelegationFilter::lookup_index_simd(const int &key) {
    const int *keys = this->keys.data();
    const int num_elements = this->size.load(std::memory_order_relaxed);

    __m128i key_vec = _mm_set1_epi32(key);
    __m128i *keys_vec = (__m128i *) keys;

    for (int i = 0; i < num_elements; i += 4) {
        __m128i comparison = _mm_cmpeq_epi32(key_vec, keys_vec[i / 4]);
        int found = _mm_movemask_epi8(comparison);
        if (found) { return __builtin_ctz(found) / 4 + i; }
    }
    return -1;
}

int DelegationFilter::lookup_value_simd(const int &key) {
    const int *keys = this->keys.data();
    const int *counts = this->counts.data();
    const int num_elements = this->size.load(std::memory_order_relaxed);

    __m128i key_vec = _mm_set1_epi32(key);
    __m128i *keys_vec = (__m128i *) keys;
    __m128i *counts_vec = (__m128i *) counts;

    for (int i = 0; i < num_elements; i += 4) {
        __m128i comparison = _mm_cmpeq_epi32(key_vec, keys_vec[i / 4]);
        int found = _mm_movemask_epi8(comparison);
        if (found) { return counts[i + __builtin_ctz(found) / 4]; }
    }
    return 0;
}

int DelegationFilter::update_or_insert_if_not_full_simd(const int &key) {
    const int *keys = this->keys.data();
    const int num_elements = this->size.load(std::memory_order_relaxed);

    __m128i key_vec = _mm_set1_epi32(key);
    __m128i *keys_vec = (__m128i *) keys;

    for (int i = 0; i < num_elements; i += 4) {
        __m128i comparison = _mm_cmpeq_epi32(key_vec, keys_vec[i / 4]);
        int found = _mm_movemask_epi8(comparison);
        if (found) { return ++counts[i + __builtin_ctz(found) / 4]; }
    }

    int size = num_elements;

    if (size < FILTER_SIZE) {
        this->keys[size] = key;
        counts[size] = 1;
        this->size.store(size + 1, std::memory_order_relaxed);
        return 1;
    }
    return 0;
}
