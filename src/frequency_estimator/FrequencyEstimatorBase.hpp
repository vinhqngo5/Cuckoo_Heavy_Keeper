#pragma once

#include <iostream>
#include <map>
#include <string>

class FrequencyEstimatorBase {
  public:
    virtual void print_status() = 0;
    virtual void update(const int &item, int c = 1) = 0;
    virtual void update(const std::string &item, int c = 1) = 0;
    virtual unsigned int estimate(const int &item) = 0;
    virtual unsigned int estimate(const std::string &item) = 0;
    virtual unsigned int update_and_estimate(const int &item, int c = 1) = 0;
    virtual unsigned int update_and_estimate(const std::string &item, int c = 1) = 0;

    // Average Relative Error (ARE)
    template <typename T> float ARE(const std::map<T, int> &exact_counter) {
        float relative_error = 0;
        for (const auto &entry : exact_counter) {
            relative_error +=
                float(abs(entry.second - (int) this->estimate(entry.first))) / entry.second;
        }
        return relative_error / exact_counter.size();
    }

    // Average Absolute Error (ARE)
    template <typename T> float AAE(const std::map<T, int> &exact_counter) {
        float absolute_error = 0;
        for (const auto &entry : exact_counter) {
            absolute_error += float(abs(entry.second - (int) this->estimate(entry.first)));
        }
        return absolute_error / exact_counter.size();
    }

    // print expected value from map's key
    template <typename T> void print_compare(const std::map<T, int> &exact_counter) {
        for (const auto &entry : exact_counter) {
            std::cout << entry.first << " estimate:" << this->estimate(entry.first)
                      << " real:" << entry.second << '\n';
        }
    }
};