#pragma once

#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"

#define LONG_PRIME 2147483647

class WeightedFrequent : public FrequencyEstimatorBase {
  public:
    // Constructor
    WeightedFrequent() = default;
    WeightedFrequent(float epsilon) : epsilon(epsilon) { n = ceil(1.0 / epsilon); }
    WeightedFrequent(int n) : n(n) { epsilon = 1.0 / n; }
    WeightedFrequent(const WeightedFrequentConfig &config) {
        if (config.CALCULATE_FROM == "EPSILON") {
            epsilon = config.EPSILON;
            n = ceil(1.0 / epsilon);
        } else if (config.CALCULATE_FROM == "N") {
            n = config.N;
            epsilon = 1.0 / n;
        }
    }

    // Update functions
    void update(const int &item, int c = 1) override { _update(item, c); }

    void update(const string &item, int c = 1) override { _update(_get_hashitem(item), c); }

    // Estimate functions
    unsigned int estimate(const int &item) override { return _estimate(item); }

    unsigned int estimate(const string &item) override { return _estimate(_get_hashitem(item)); }

    // Update and estimate functions
    unsigned int update_and_estimate(const int &item, int c = 1) override {
        _update(item, c);
        return _estimate(item);
    }

    unsigned int update_and_estimate(const string &item, int c = 1) override {
        unsigned int hashitem = _get_hashitem(item);
        _update(hashitem, c);
        return _estimate(hashitem);
    }

    // Print status function
    void print_status() override {
        // Implementation depends on what you want to print
    }

  private:
    float epsilon;
    int n;
    std::unordered_map<int, int> T;

    unsigned int _get_hashitem(const string &item) {
        unsigned long hash = 5381;
        for (char c : item) { hash = (((hash << 5) + hash) + c) % LONG_PRIME; /* hash * 33 + c */ }
        return hash;
    }

    void _update(const int &item, int c) {
        if (T.find(item) != T.end()) {
            T[item] += c;
        } else if (T.size() < n) {
            T[item] = c;
        } else {
            int c_min = std::min_element(T.begin(), T.end(), [](const auto &a, const auto &b) { return a.second < b.second; })->second;

            if (c >= c_min) {
                for (auto it = T.begin(); it != T.end();) {
                    it->second -= c_min;
                    if (it->second <= 0) {
                        it = T.erase(it);
                    } else {
                        ++it;
                    }
                }
                T[item] = c - c_min;
            } else {
                for (auto &pair : T) { pair.second -= c; }
            }
        }
    }

    unsigned int _estimate(const int &item) { return T.find(item) != T.end() ? T[item] : 0; }
};