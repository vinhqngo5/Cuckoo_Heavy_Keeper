#pragma once

#include "StreamSummary.hpp"
#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "hash/BOBHash32.hpp"
#include "hash/BOBHash64.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#define rep(i, a, n) for (int i = a; i <= n; i++)

constexpr int N = 1000000;      // maximum flow
constexpr int M = 1000000;      // maximum size of stream-summary or CSS
constexpr int MAX_MEM = 2000;   // maximum memory size

using std::string, std::pair, std::make_pair, std::sort;

class StreamSummarySpaceSaving : public FrequencyEstimatorBase {
  public:
    int total = 0;
    StreamSummarySpaceSaving(int M2, int K);

    StreamSummarySpaceSaving(SpaceSavingConfig &config);

    void update(const std::string &x, int c = 1) override;

    void update(const int &x, int c = 1) override;

    unsigned int estimate(const std::string &item) override;

    unsigned int estimate(const int &item) override;

    unsigned int update_and_estimate(const std::string &item, int c = 1) override;

    unsigned int update_and_estimate(const int &item, int c = 1) override;

    void print_status() override;

  private:
    StreamSummary *ss;
    int K, M2;

    void _assert_not_implemented(int c);
};
