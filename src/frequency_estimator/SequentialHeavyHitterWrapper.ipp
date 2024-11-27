
// sequential_heavy_hitter_wrapper.ipp
#pragma once

template <typename FrequencyEstimator, typename T>
void SequentialHeavyHitterWrapper<FrequencyEstimator, T>::check_and_update_heavy_hitter(const std::string &item, unsigned int item_count) {
    if (item_count > theta * total) { pq_heavy_hitters.update(item, item_count); }
}

template <typename FrequencyEstimator, typename T>
void SequentialHeavyHitterWrapper<FrequencyEstimator, T>::check_and_update_heavy_hitter(const int &item, unsigned int item_count) {
    if (item_count > theta * total) { pq_heavy_hitters.update(item, item_count); }
}

template <typename FrequencyEstimator, typename T>
SequentialHeavyHitterWrapper<FrequencyEstimator, T>::SequentialHeavyHitterWrapper(FrequencyEstimator &frequency_estimator, float theta, size_t max_heavy_hitters)
    : frequency_estimator(frequency_estimator), pq_heavy_hitters(max_heavy_hitters), theta(theta), total(0) {}

template <typename FrequencyEstimator, typename T> void SequentialHeavyHitterWrapper<FrequencyEstimator, T>::print_status() {
    frequency_estimator.print_status();
    std::cout << "Heavy Hitters (theta = " << theta << "):\n";
    std::cout << pq_heavy_hitters << std::endl;
}

template <typename FrequencyEstimator, typename T> void SequentialHeavyHitterWrapper<FrequencyEstimator, T>::update(const int &item, int c) {
    total += c;
    unsigned int item_count = frequency_estimator.update_and_estimate(item, c);
    check_and_update_heavy_hitter(item, item_count);
}

template <typename FrequencyEstimator, typename T> void SequentialHeavyHitterWrapper<FrequencyEstimator, T>::update(const std::string &item, int c) {
    total += c;
    unsigned int item_count = frequency_estimator.update_and_estimate(item, c);
    check_and_update_heavy_hitter(item, item_count);
}

template <typename FrequencyEstimator, typename T> unsigned int SequentialHeavyHitterWrapper<FrequencyEstimator, T>::estimate(const int &item) {
    return frequency_estimator.estimate(item);
}

template <typename FrequencyEstimator, typename T> unsigned int SequentialHeavyHitterWrapper<FrequencyEstimator, T>::estimate(const std::string &item) {
    return frequency_estimator.estimate(item);
}

template <typename FrequencyEstimator, typename T> unsigned int SequentialHeavyHitterWrapper<FrequencyEstimator, T>::update_and_estimate(const int &item, int c) {
    total += c;
    unsigned int item_count = frequency_estimator.update_and_estimate(item, c);
    check_and_update_heavy_hitter(item, item_count);
    return item_count;
}

template <typename FrequencyEstimator, typename T> unsigned int SequentialHeavyHitterWrapper<FrequencyEstimator, T>::update_and_estimate(const std::string &item, int c) {
    total += c;
    unsigned int item_count = frequency_estimator.update_and_estimate(item, c);
    check_and_update_heavy_hitter(item, item_count);
    return item_count;
}

// template <typename FrequencyEstimator, typename T> const BoundedKeyValuePriorityQueue<T> &SequentialHeavyHitterWrapper<FrequencyEstimator, T>::get_heavy_hitters() const {
//     return pq_heavy_hitters;
// }

template <typename FrequencyEstimator, typename T> const BoundedKeyValuePriorityQueue<T> &SequentialHeavyHitterWrapper<FrequencyEstimator, T>::get_heavy_hitters() const {
    if constexpr (std::is_same_v<FrequencyEstimatorConfig, SpaceSavingConfig>) {
        // Clear existing queue
        pq_heavy_hitters = BoundedKeyValuePriorityQueue<T>();

        // Iterate through space saving array
        for (const auto &item : frequency_estimator) {
            int freq = item->count;
            if (freq >= theta) {
                if constexpr (std::is_same_v<T, std::string>) {
                    pq_heavy_hitters.push(item->name, freq);
                } else {
                    pq_heavy_hitters.push(std::stoi(item->name), freq);
                }
            }
        }
    }
    return pq_heavy_hitters;
}
template <typename FrequencyEstimator, typename T> float SequentialHeavyHitterWrapper<FrequencyEstimator, T>::get_theta() const { return theta; }

template <typename FrequencyEstimator, typename T> unsigned long SequentialHeavyHitterWrapper<FrequencyEstimator, T>::get_count() const { return total; }