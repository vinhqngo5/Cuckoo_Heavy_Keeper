#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "frequency_estimator/CountMinSketch.hpp"
#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"

struct AugmentedFilter {
    std::vector<int> keys;
    std::vector<int> counts;
    std::vector<int> old_counts;
    int size;
    int max_size;
    int min_count;

    AugmentedFilter();
    AugmentedFilter(int max_size);

    void init(int max_size);
};

class AugmentedSketch : public FrequencyEstimatorBase {
  public:
    int total;

    AugmentedSketch();
    AugmentedSketch(unsigned int width, unsigned int depth, int max_filter_size);
    AugmentedSketch(float epsilon, float delta, int max_filter_size);
    AugmentedSketch(AugmentedSketchConfig augmented_configs);

    void update(const int &item, int c = 1) override;
    void update(const std::string &item, int c = 1);

    unsigned int estimate(const int &item) override;
    unsigned int estimate(const std::string &item) override;

    unsigned int update_and_estimate(const int &item, int c = 1);
    unsigned int update_and_estimate(const std::string &item, int c = 1);

    void print_status();

  private:
    CountMinSketch count_min_sketch;
    AugmentedFilter augmented_filter;
    void _update(const int item, int c = 1);
    unsigned int _estimate(const int &item);
    unsigned int _get_hashitem(const std::string &item);
};
