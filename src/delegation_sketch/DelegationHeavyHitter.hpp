#pragma once
#include <emmintrin.h>

#include <atomic>
#include <barrier>
#include <condition_variable>
#include <filesystem>
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

// global DelegationSketchContext, used to pass the context to the all threads
struct DelegationSketchContext {
    AppConfig app_configs;
    DelegationHeavyHitterConfig delegation_configs;
    Relation *r1;   // input data
    std::atomic<bool> &START_BENCHMARK;
    std::atomic<bool> &START_ACCURACY_EVALUATION;

    DelegationSketchContext(AppConfig app_configs, DelegationHeavyHitterConfig delegation_configs, Relation *r1, std::atomic<bool> &START_BENCHMARK,
                            std::atomic<bool> &START_ACCURACY_EVALUATION)
        : app_configs(app_configs), delegation_configs(delegation_configs), r1(r1), START_BENCHMARK(START_BENCHMARK), START_ACCURACY_EVALUATION(START_ACCURACY_EVALUATION) {}
};

// GlobalHeavyHitterTracker
struct GlobalHeavyHitterTracker {
    DelegationSketchContext &delegation_sketch_context;
    libcuckoo::cuckoohash_map<int, int> global_heavy_hitters;
    atomic<int> stream_size = 0;

    GlobalHeavyHitterTracker(DelegationSketchContext &delegation_sketch_context) : delegation_sketch_context(delegation_sketch_context) {}
};

// LocalHeavyHitterTracker
struct LocalHeavyHitterTracker {
    DelegationSketchContext &delegation_sketch_context;
    BoundedKeyValuePriorityQueue<int> local_heavy_hitters;
    vector<tuple<int, int, int>> local_heavy_hitter_differences;
    int threshold = 0;

    LocalHeavyHitterTracker(DelegationSketchContext &delegation_sketch_context) : delegation_sketch_context(delegation_sketch_context) {}

    bool add_if_is_local_heavy_hitter(int key, int difference, int count);
    void update_threshold(int threshold);
    void update_global_heavy_hitters(GlobalHeavyHitterTracker &global_heavy_hitter_tracker, int total_differences);
};

// Declare Thread-Local Delegation Sketch
template <typename FrequencyEstimator> class ThreadLocalDelegationHeavyHitter;

// DelegationHeavyHitter
template <typename FrequencyEstimator> class DelegationHeavyHitter {
  public:
    DelegationSketchContext &delegation_sketch_context;
    std::vector<ThreadLocalDelegationHeavyHitter<FrequencyEstimator> *> thread_local_delegation_sketches;
    map<int, int> accuracy_evaluator_heavy_hitter_counter;
    vector<int> latency_evaluator_cache;
    GlobalHeavyHitterTracker global_heavy_hitter_tracker;
    atomic<int> QPOPSS_stream_size = 0;

    DelegationHeavyHitter(DelegationSketchContext &delegation_sketch_context, std::vector<FrequencyEstimator> &frequency_estimators);
    DelegationHeavyHitter() = default;

    int direct_query(const int &key);
    void query_all_heavy_hitters(map<int, int> &results);
    template <typename T> float ARE(const std::map<T, int> &exact_counter);
    template <typename T> float AAE(const std::map<T, int> &exact_counter);
    template <typename T> void print_compare(const std::map<T, int> &exact_counter, string output_file_path = "");
};

// Thread-Local Delegation Sketch
template <typename FrequencyEstimator> class ThreadLocalDelegationHeavyHitter {
  public:
    DelegationSketchContext &delegation_sketch_context;
    FrequencyEstimator &frequency_estimator;
    std::vector<PendingQuery *> pending_queries;
    std::vector<DelegationFilter *> delegation_filters;
    std::array<std::vector<DelegationFilter *>, 2> double_buffer_delegation_filters;
    std::vector<int> current_buffer_ids;

    std::vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors;
    ThreadOverallStatCollector thread_overall_stat_collector;

    LCRQueue<DelegationFilter *> full_delegate_filters;
    DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch;
    LocalHeavyHitterTracker local_heavy_hitter_tracker;
    int current_thread_id;
    unsigned long *seeds;
    std::mutex QPOPSS_mutex;

    ThreadLocalDelegationHeavyHitter(DelegationSketchContext &delegation_sketch_context, int current_thread_id, FrequencyEstimator &frequency_estimator,
                                     DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch);

    void process_pending_inserts();
    void process_pending_queries();
    void flush_pending_inserts();
    void insert(const int &key);
    int query(const int &key);
    void query_all_heavy_hitters(map<int, int> &results);
    void insert_directly(const int &key);
    int query_directly(const int &key);
};

template <typename FrequencyEstimator>
void start_thread_heavy_hitter(DelegationSketchContext &delegation_sketch_context, ThreadLocalDelegationHeavyHitter<FrequencyEstimator> *thread_local_delegation_sketch, int start,
                               int end, std::barrier<> &sync_point);

template <typename FrequencyEstimator>
void start_accuracy_evaluator(DelegationSketchContext &delegation_sketch_context, DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch,
                              map<int, int> &accuracy_evaluator_heavy_hitter_counter, std::barrier<> &sync_point);

template <typename FrequencyEstimator>
DelegationHeavyHitter<FrequencyEstimator> *start_threads(DelegationSketchContext &delegation_sketch_context, vector<FrequencyEstimator> &frequency_estimators);

template <typename FrequencyEstimator>
pair<map<int, int>, long> calculate_exact_counter(DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch, Relation *r1, int num_threads, int tuples_no);

template <typename FrequencyEstimator>
void print_stats_for_delegation_sketch(DelegationSketchContext &delegation_sketch_context, DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch,
                                       string output_file_path = "");

template <typename FrequencyEstimator, typename KeyType>
void print_stats_for_heavy_hitters(DelegationSketchContext &delegation_sketch_context, DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch, string output_file_path = "");

string create_file_path_from_context(DelegationSketchContext &delegation_sketch_context, string suffix = "") {
    auto now = std::chrono::system_clock::now();
    auto time_start = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_start), "+%F_%H%M%S");

    string slug = "/experiments/ALGORITHM=" + std::string(DelegationBuildConfig::algorithm) + "/MODE=" + std::string(DelegationBuildConfig::mode) +
                  "/PARARLLEL_DESIGN=" + std::string(DelegationBuildConfig::parallel_design) + "/EVALUATE_MODE=" + std::string(DelegationBuildConfig::evaluate_mode) +
                  "/EVALUATE_ACCURACY_WHEN=" + std::string(DelegationBuildConfig::evaluate_accuracy_when) +
                  "/EVALUATE_ACCURACY_ERROR_SOURCES=" + std::string(DelegationBuildConfig::evaluate_accuracy_error_sources) +
                  "/EVALUATE_ACCURACY_STREAM_SIZE=" + std::to_string(DelegationBuildConfig::evaluate_accuracy_stream_size) +
                  "/NUM_THREADS=" + to_string(delegation_sketch_context.app_configs.NUM_THREADS) + "/DIST_PARAM=" + to_string(delegation_sketch_context.app_configs.DIST_PARAM) +
                  "/THETA=" + to_string(delegation_sketch_context.app_configs.THETA) +
                  "/HEAVY_QUERY_RATE=" + to_string(delegation_sketch_context.delegation_configs.HEAVY_QUERY_RATE) + +"/";

    // Get the current path of the source file
    string current_path = __FILE__;
    // Find the position of the "src" directory in the current path
    size_t pos = current_path.find("/src/");
    // Construct the filename using the project root and the desired relative path
    string filename = current_path.substr(0, pos) + slug + ss.str() + suffix + ".json";

    // Create the directory if it doesn't exist
    std::filesystem::path dir_path = std::filesystem::path(filename).parent_path();
    std::filesystem::create_directories(dir_path);

    cout << "filename: " << filename << endl;
    return filename;
};

#include "DelegationHeavyHitter.ipp"
