#pragma once
#include <map>
#include <vector>

template <typename FrequencyEstimator, typename KeyType> std::map<KeyType, int> get_exact_counter(const std::vector<KeyType> &data, int line_read) {
    std::map<KeyType, int> exact_counter;
    for (int i = 0; i < line_read; i++) { exact_counter[data[i]] += 1; }
    return exact_counter;
}