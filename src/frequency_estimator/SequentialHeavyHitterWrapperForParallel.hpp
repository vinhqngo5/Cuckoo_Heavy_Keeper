
// sequential_heavy_hitter_wrapper.hpp
#pragma once

#include "BoundedKeyValuePriorityQueue.hpp"
#include "FrequencyEstimatorTrait.hpp"
#include <memory>
#include <string>

template <typename FrequencyEstimator, typename T> class SequentialHeavyHitterWrapperForParallel {
    using FrequencyEstimatorConfig = typename FrequencyEstimatorConfigTrait<FrequencyEstimator>::type;

  private:
    FrequencyEstimator &frequency_estimator;
    mutable BoundedKeyValuePriorityQueue<T> pq_heavy_hitters;
    float theta;

    void check_and_update_heavy_hitter(const std::string &item, unsigned int item_count);
    void check_and_update_heavy_hitter(const int &item, unsigned int item_count);

  public:
    unsigned long total;
    int threshold = 0;
    SequentialHeavyHitterWrapperForParallel(FrequencyEstimator &frequency_estimator, float theta, size_t max_heavy_hitters = 0);

    void print_status();
    void update(const int &item, int c = 1);
    void update(const std::string &item, int c = 1);
    void update_threshold(int threshold);
    unsigned int estimate(const int &item);
    unsigned int estimate(const std::string &item);
    unsigned int update_and_estimate(const int &item, int c = 1);
    unsigned int update_and_estimate(const std::string &item, int c = 1);

    // Helper methods
    const BoundedKeyValuePriorityQueue<T> &get_heavy_hitters() const;
    float get_theta() const;
    unsigned long get_count() const;
};

#include "SequentialHeavyHitterWrapperForParallel.ipp"