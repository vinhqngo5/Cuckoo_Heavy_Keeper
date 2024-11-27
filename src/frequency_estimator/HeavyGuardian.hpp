#pragma once

#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "frequency_estimator/MacroPreprocessor.hpp"
#include "frequency_estimator/StreamSummary.hpp"
#include "hash/BOBHash32.hpp"
#include "hash/BOBHash64.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

constexpr int G = 8;
constexpr int ct = 16;
constexpr double HK_b = 1.08;
using std::string;

class HeavyGuardian : public FrequencyEstimatorBase {
  public:
    int total = 0;
    HeavyGuardian(int M) : M(M) {
        // generate random number between 0 and 1228
        srand((unsigned int) clock());
        int random_seed = rand() % 1228;
        bobhash = new BOBHash64(random_seed);
    }
    void print_status();
    void update(const std::string &item, int c = 1) override;
    void update(const int &item, int c = 1) override;
    unsigned int estimate(const std::string &item) override;
    unsigned int estimate(const int &item) override;
    unsigned int update_and_estimate(const std::string &item, int c = 1) override;
    unsigned int update_and_estimate(const int &item, int c = 1) override;

  private:
    struct node {
        int C;
        unsigned int FP;
    } HK[10005][20];
    int ext[10005][40];
    BOBHash64 *bobhash;
    int M;
    void _insert(const std::string &item);
    unsigned int _estimate(const std::string &item);
    void _assert_not_implemented(int c);
    unsigned long long hash(std::string ST);
};
