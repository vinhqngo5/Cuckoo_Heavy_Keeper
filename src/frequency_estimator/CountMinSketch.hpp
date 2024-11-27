#pragma once

#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <string>

class CountMinSketch : public FrequencyEstimatorBase {
  public:
    // width, depth
    unsigned int width, depth;

    // epsilon, delta
    // eps (for error), 0.01 < eps < 1 -> the smaller the better
    // delta (probability for accuracy), 0 < gamma < 1 -> the bigger the better
    float epsilon, delta;
    unsigned int total = 0;

    // declare function prototypes
    void init_countmin_sketch();
    void genajbj(int **, int);

    // constructors
    CountMinSketch();
    CountMinSketch(unsigned int, unsigned int);
    CountMinSketch(float, float);
    CountMinSketch(const CountMinConfig &);

    // update and estimate functions (override from base class)
    void update(const int &, int = 1) override;
    void update(const string &, int = 1) override;
    unsigned int estimate(const int &) override;
    unsigned int estimate(const string &) override;
    unsigned int update_and_estimate(const int &, int = 1) override;
    unsigned int update_and_estimate(const string &, int = 1) override;

    // Utility function
    template <typename T> float correct_probability(const map<T, int> &);

    // print function (override from base class)
    void print_status() override;

  private:
    // internal variables
    int **C;
    unsigned int aj, bj;
    int **hashes;

    // internal update function
    void _update(const int &, int = 1);
    unsigned int _estimate(const int &);
    unsigned int _get_hashitem(const string &);
};

template <typename T> float CountMinSketch::correct_probability(const map<T, int> &exact_counter) {
    int count_wrong = 0;
    for (const auto &entry : exact_counter) {
        if (entry.second + this->total * this->epsilon < this->estimate(entry.first)) { count_wrong += 1; }
    }
    // cout << "Count wrong: " << count_wrong << endl;
    return 1 - (float) count_wrong / exact_counter.size();
}