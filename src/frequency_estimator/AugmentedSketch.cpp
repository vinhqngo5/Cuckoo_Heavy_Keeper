#include "AugmentedSketch.hpp"

#define LONG_PRIME 2147483647

// AugmentedFilter Constructors
AugmentedFilter::AugmentedFilter() {}

AugmentedFilter::AugmentedFilter(int max_size) {
    this->max_size = max_size;
    keys = std::vector<int>(max_size);
    counts = std::vector<int>(max_size);
    old_counts = std::vector<int>(max_size);
    size = 0;
}

void AugmentedFilter::init(int max_size) {
    this->max_size = max_size;
    keys = std::vector<int>(max_size);
    counts = std::vector<int>(max_size);
    old_counts = std::vector<int>(max_size);
    size = 0;
}

// AugmentedSketch Constructors
AugmentedSketch::AugmentedSketch() {}

AugmentedSketch::AugmentedSketch(unsigned int width, unsigned int depth, int max_filter_size) {
    count_min_sketch = CountMinSketch(width, depth);
    augmented_filter.init(max_filter_size);
}

AugmentedSketch::AugmentedSketch(float epsilon, float delta, int max_filter_size) {
    count_min_sketch = CountMinSketch(epsilon, delta);
    augmented_filter.init(max_filter_size);
}

AugmentedSketch::AugmentedSketch(AugmentedSketchConfig augmented_configs) {
    count_min_sketch = CountMinSketch(augmented_configs);
    augmented_filter.init(augmented_configs.FILTER_SIZE);
}

// Hash item for strings
unsigned int AugmentedSketch::_get_hashitem(const std::string &item) {
    unsigned long hash = 5381;
    for (char c : item) { hash = (((hash << 5) + hash) + c) % LONG_PRIME; /* hash * 33 + c */ }
    return hash;
}

// Update function for integers
void AugmentedSketch::update(const int &item, int c) { return this->_update(item, c); }

// Update function for strings
void AugmentedSketch::update(const std::string &item, int c) {
    unsigned int hashitem = this->_get_hashitem(item);
    return this->_update(hashitem, c);
}

// Internal update logic
void AugmentedSketch::_update(const int item, int c) {
    this->total += c;

    // Check if key is in the filter
    for (int j = 0; j < augmented_filter.size; ++j) {
        if (augmented_filter.keys[j] == item) {
            augmented_filter.counts[j] += c;
            return;
        }
    }

    // If key not in filter and filter not full
    if (augmented_filter.size < augmented_filter.max_size) {
        augmented_filter.keys[augmented_filter.size] = item;
        augmented_filter.counts[augmented_filter.size] += c;
        augmented_filter.old_counts[augmented_filter.size] = 0;
        augmented_filter.size++;
        return;
    }

    // If key not in filter and filter is full
    int estimated_count = this->count_min_sketch.update_and_estimate(item, c);

    int min_index = 0, min_count = augmented_filter.counts[0];
    for (int j = 1; j < augmented_filter.size; ++j) {
        if (augmented_filter.counts[j] < min_count) {
            min_count = augmented_filter.counts[j];
            min_index = j;
        }
    }

    if (min_count < estimated_count) {
        int drift = augmented_filter.counts[min_index] - augmented_filter.old_counts[min_index];
        if (drift > 0) { this->count_min_sketch.update(augmented_filter.keys[min_index], drift); }
        augmented_filter.keys[min_index] = item;
        augmented_filter.counts[min_index] = estimated_count;
        augmented_filter.old_counts[min_index] = estimated_count;
    }
}

// Estimate function for integers
unsigned int AugmentedSketch::estimate(const int &item) { return this->_estimate(item); }

// Estimate function for strings
unsigned int AugmentedSketch::estimate(const std::string &item) {
    unsigned int hashitem = this->_get_hashitem(item);
    return this->_estimate(hashitem);
}

// Internal estimate logic
unsigned int AugmentedSketch::_estimate(const int &item) {
    for (int j = 0; j < augmented_filter.size; ++j) {
        if (augmented_filter.keys[j] == item) { return augmented_filter.counts[j]; }
    }

    return this->count_min_sketch.estimate(item);
}

// Update and estimate function for integers
unsigned int AugmentedSketch::update_and_estimate(const int &item, int c) {
    // Update total count
    this->total += c;

    // Check if item is in the filter
    for (int j = 0; j < augmented_filter.size; ++j) {
        if (augmented_filter.keys[j] == item) {
            augmented_filter.counts[j] += c;
            return augmented_filter.counts[j];
        }
    }

    // If key not in filter and filter not full
    if (augmented_filter.size < augmented_filter.max_size) {
        augmented_filter.keys[augmented_filter.size] = item;
        augmented_filter.counts[augmented_filter.size] += c;
        augmented_filter.old_counts[augmented_filter.size] = 0;
        augmented_filter.size++;
        return c;
    }

    // If key not in filter and filter is full
    int estimated_count = this->count_min_sketch.update_and_estimate(item, c);

    int min_index = 0, min_count = augmented_filter.counts[0];
    for (int j = 1; j < augmented_filter.size; ++j) {
        if (augmented_filter.counts[j] < min_count) {
            min_count = augmented_filter.counts[j];
            min_index = j;
        }
    }

    if (min_count < estimated_count) {
        int drift = augmented_filter.counts[min_index] - augmented_filter.old_counts[min_index];
        if (drift > 0) { this->count_min_sketch.update(augmented_filter.keys[min_index], drift); }
        augmented_filter.keys[min_index] = item;
        augmented_filter.counts[min_index] = estimated_count;
        augmented_filter.old_counts[min_index] = estimated_count;
        return estimated_count;
    }

    return estimated_count;
}

// Update and estimate function for strings
unsigned int AugmentedSketch::update_and_estimate(const std::string &item, int c) {
    unsigned int hashitem = this->_get_hashitem(item);
    return this->update_and_estimate(hashitem, c);
}

// Print status function
void AugmentedSketch::print_status() {
    // Implementation of print status (if needed)
}
