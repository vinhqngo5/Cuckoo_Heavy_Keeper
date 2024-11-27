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

class HeavyKeeper : public FrequencyEstimatorBase {
  public:
    constexpr static int HK_d = 2;
    constexpr static double HK_b = 1.08;
    constexpr static int N = 1000000;      // maximum flow
    constexpr static int M = 1000000;      // maximum size of stream-summary or CSS
    constexpr static int MAX_MEM = 2000;   // maximum memory size
    struct node {
        int C = 0, FP = 0;
    };

    unsigned int total = 0;   // total count so far

    HeavyKeeper(int M2, int K);
    HeavyKeeper(int M2);
    HeavyKeeper(HeavyKeeperConfig &config);
    void generate_new_seed();
    void clear();

    void work();
    std::pair<std::string, int> query(const int &k);
    void print_status();
    void update(const std::string &item, int c = 1) override;
    void update(const int &item, int c = 1) override;
    unsigned int estimate(const std::string &item) override;
    unsigned int estimate(const int &item) override;
    unsigned int update_and_estimate(const std::string &item, int c = 1) override;
    unsigned int update_and_estimate(const int &item, int c = 1) override;

  private:
    StreamSummary *ss;
    node HK[HK_d][MAX_MEM + 10];
    BOBHash64 *bobhash;
    int K, M2;

    struct Node {
        std::string x;
        int y;
    } q[MAX_MEM + 10];

    static int cmp(Node i, Node j) { return i.y > j.y; }

    // internal update function
    void _insert_with_StreamSummary(const std::string &x);
    void _insert(const std::string &x);
    unsigned int _estimate(const std::string &item);
    unsigned long long hash(const std::string &ST);
    void _assert_not_implemented(int c);
};
