#pragma once
#include <emmintrin.h>

#include <atomic>
#include <barrier>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "concurrent_data_structure/LCRQueue.hpp"
#include "concurrent_data_structure/TreiberStack.hpp"
#include "concurrent_data_structure/libcuckoo/cuckoohash_map.hh"
#include "delegation_sketch/DelegationConfig.hpp"
#include "delegation_sketch/DelegationFilter.hpp"
#include "delegation_sketch/PendingQuery.hpp"
#include "delegation_sketch/StatCollector.hpp"
#include "delegation_sketch/delegation_sketch_utils.hpp"
#include "frequency_estimator/AugmentedSketch.hpp"
#include "frequency_estimator/BoundedKeyValuePriorityQueue.hpp"
#include "frequency_estimator/CuckooHeavyKeeper.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "heavy_hitter_app/AppConfig.hpp"
#include "heavy_hitter_app/Relation.hpp"
#include "utils/getticks.hpp"

using AppConfig = ParallelAppConfig;

// Declare Thread-Local Delegation Sketch
template <typename FrequencyEstimator> class ThreadLocalDelegationSketch;

// DelegationSketch and DelegationHeavyHitter
template <typename FrequencyEstimator> class DelegationSketch {
  public:
    int num_threads;
    int filter_size;
    DelegationConfig delegation_configs;
    std::vector<ThreadLocalDelegationSketch<FrequencyEstimator> *> thread_local_delegation_sketches;
    DelegationSketch(ParallelAppConfig app_configs, DelegationConfig delegation_configs, std::vector<FrequencyEstimator> &frequency_estimators);
    DelegationSketch() = default;

    int direct_query(const int &key);

    template <typename T> float ARE(const std::map<T, int> &exact_counter);

    template <typename T> float AAE(const std::map<T, int> &exact_counter);
};

// Thread-Local Delegation Sketch
template <typename FrequencyEstimator> class ThreadLocalDelegationSketch {
  public:
    FrequencyEstimator frequency_estimator;
    std::vector<PendingQuery *> pending_queries;
    std::vector<DelegationFilter *> delegation_filters;
    std::array<std::vector<DelegationFilter *>, 2> double_buffer_delegation_filters;
    std::vector<int> current_buffer_ids;
    atomic<bool> START_BENCHMARK;

    std::vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors;
    ThreadOverallStatCollector thread_overall_stat_collector;
    DelegationConfig delegation_configs;

    LCRQueue<DelegationFilter *> full_delegate_filters;
    DelegationSketch<FrequencyEstimator> *delegation_sketch;
    int current_thread_id;
    long long element_processed_during_time_interval = 0, query_processed_during_time_interval = 0;
    unsigned long *seeds;
    int FILTER_SIZE;

    ThreadLocalDelegationSketch(ParallelAppConfig app_configs, DelegationConfig delegation_configs, int current_thread_id, FrequencyEstimator frequency_estimator,
                                DelegationSketch<FrequencyEstimator> *delegation_sketch);

    void process_pending_inserts();
    void process_pending_queries();
    void flush_pending_inserts();
    void insert(const int &key);
    int query(const int &key);
    void insert_directly(const int &key);
    int query_directly(const int &key);
};

template <typename FrequencyEstimator>
void start_thread(DelegationConfig delegation_configs, ThreadLocalDelegationSketch<FrequencyEstimator> *thread_local_delegation_sketch, int start, int end, Relation *r1,
                  std::barrier<> &sync_point, atomic<bool> &START_BENCHMARK);

template <typename FrequencyEstimator>
DelegationSketch<FrequencyEstimator> *start_threads(AppConfig app_configs, DelegationBuildConfig delegation_build_configs, DelegationConfig delegation_configs, Relation *r1,
                                                    vector<FrequencyEstimator> &frequency_estimators, atomic<bool> &START_BENCHMARK);

template <typename FrequencyEstimator> void print_stats_for_delegation_sketch(AppConfig app_configs, DelegationSketch<FrequencyEstimator> *delegation_sketch);

#include "DelegationSketch.ipp"