#pragma once
#include <atomic>
#include <emmintrin.h>
#include <vector>

// Delegation Filter
struct DelegationFilter {
    std::vector<int> keys;
    std::vector<int> counts;
    std::atomic<int> size;
    std::atomic<bool> lock;
    int FILTER_SIZE;

    DelegationFilter();
    DelegationFilter(int max_size);
    DelegationFilter(DelegationFilter &&other);

    int lookup_value(const int &key);
    int lookup_index(const int &key);
    int update_or_insert_if_not_full(const int &key);
    int lookup_index_simd(const int &key);
    int lookup_value_simd(const int &key);
    int update_or_insert_if_not_full_simd(const int &key);
};