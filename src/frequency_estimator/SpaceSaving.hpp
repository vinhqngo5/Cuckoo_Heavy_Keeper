#pragma once

#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class SpaceSaving : public FrequencyEstimatorBase {
  private:
    int k;
    std::vector<std::string> items;
    std::vector<unsigned int> counts;

    std::string int_to_string(const int &item);

    void _update(const std::string &item, int c);

  public:
    SpaceSaving(int k);

    SpaceSaving(SpaceSavingConfig &config);

    void print_status() override;

    void update(const int &item, int c = 1) override;

    void update(const std::string &item, int c = 1) override;

    unsigned int estimate(const int &item) override;

    unsigned int estimate(const std::string &item) override;

    unsigned int update_and_estimate(const int &item, int c = 1) override;

    unsigned int update_and_estimate(const std::string &item, int c = 1) override;
};
