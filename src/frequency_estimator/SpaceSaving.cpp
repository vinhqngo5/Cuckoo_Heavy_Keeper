#include "SpaceSaving.hpp"
#include <algorithm>

SpaceSaving::SpaceSaving(int k) : k(k) {}

SpaceSaving::SpaceSaving(SpaceSavingConfig &config) : k(config.K) {}

std::string SpaceSaving::int_to_string(const int &item) { return std::to_string(item); }

void SpaceSaving::_update(const std::string &item, int c) {
    auto it = std::find(items.begin(), items.end(), item);
    if (it != items.end()) {
        counts[it - items.begin()] += c;
    } else if (items.size() < k) {
        items.push_back(item);
        counts.push_back(c);
    } else {
        auto min_it = std::min_element(counts.begin(), counts.end());
        size_t min_index = min_it - counts.begin();
        items[min_index] = item;
        counts[min_index] += c;
    }
}

void SpaceSaving::print_status() {
    std::cout << "Size: " << k << std::endl;
    std::cout << "Estimated size in bytes: " << k * (sizeof(std::string) + sizeof(unsigned int)) << std::endl;
}

void SpaceSaving::update(const int &item, int c) { _update(int_to_string(item), c); }

void SpaceSaving::update(const std::string &item, int c) { _update(item, c); }

unsigned int SpaceSaving::estimate(const int &item) { return estimate(int_to_string(item)); }

unsigned int SpaceSaving::estimate(const std::string &item) {
    auto it = std::find(items.begin(), items.end(), item);
    return (it != items.end()) ? counts[it - items.begin()] : 0;
}

unsigned int SpaceSaving::update_and_estimate(const int &item, int c) { return update_and_estimate(int_to_string(item), c); }

unsigned int SpaceSaving::update_and_estimate(const std::string &item, int c) {
    auto it = std::find(items.begin(), items.end(), item);
    if (it != items.end()) {
        counts[it - items.begin()] += c;
        return counts[it - items.begin()];
    } else if (items.size() < k) {
        items.push_back(item);
        counts.push_back(c);
        return c;
    } else {
        auto min_it = std::min_element(counts.begin(), counts.end());
        size_t min_index = min_it - counts.begin();
        items[min_index] = item;
        counts[min_index] += c;
        return counts[min_index];
    }
}
