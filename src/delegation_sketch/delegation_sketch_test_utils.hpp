#include "delegation_sketch/DelegationConfig.hpp"
#include "frequency_estimator/FrequencyEstimatorTrait.hpp"
#include "heavy_hitter_app/AppConfig.hpp"
#include "heavy_hitter_app/heavy_hitter_test_utils.hpp"
#include <barrier>
#include <iostream>
#include <map>
#include <string>
#include <thread>

extern atomic<bool> START_BENCHMARK;

template <typename FrequencyEstimator, typename KeyType>
void run_worker_thread(const PRIF<FrequencyEstimator, KeyType> &prif, ThreadLocalPRIF<FrequencyEstimator, KeyType> &thread_local_prif, int start, int end,
                       const std::vector<KeyType> &data, std::barrier<> &sync_point) {
    setaffinity_oncpu(thread_local_prif.current_thread_id + 2);
    sync_point.arrive_and_wait();

    while (START_BENCHMARK.load(std::memory_order_relaxed)) {
        for (int i = start; i < end; i++) {
            // ...
        }
    }
}

template <typename FrequencyEstimatorConfig, typename KeyType>
void run_test(ParallelAppConfig app_configs, DelegationConfig delegation_configs, FrequencyEstimatorConfig frequency_estimator_configs, const std::vector<KeyType> &data) {
    using FrequencyEstimator = typename FrequencyEstimatorTrait<FrequencyEstimatorConfig>::type;

    int line_read = app_configs.LINE_READ;
    float theta = app_configs.THETA;
    int num_runs = app_configs.NUM_RUNS;
    int num_threads = app_configs.NUM_THREADS;
    int DURATION = app_configs.DURATION;

    std::map<KeyType, int> exact_counter = get_exact_counter<KeyType>(data, line_read);

    std::map<KeyType, int> heavy_hitter_counter;
    float threshold = theta * line_read;
    for (const auto &entry : exact_counter) {
        if (entry.second >= threshold) { heavy_hitter_counter[entry.first] = entry.second; }
    }

    for (int i = 0; i < num_runs; i++) {
        std::cout << "## Run " << i << std::endl;
        std::barrier sync_point(num_threads + 1);
        sync_point.arrive_and_wait();

        // ....

        // start benchmark
        if (DURATION > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(DURATION));
            START_BENCHMARK.store(false, std::memory_order_relaxed);
            stop_time();
        }

        // join threads
        for (auto &worker_thread : worker_threads) { worker_thread.join(); }
    }
}

template <typename FrequencyEstimatorConfig>
void test_CAIDA(ParallelAppConfig &app_configs, DelegationConfig &delegation_configs, FrequencyEstimatorConfig &frequency_estimator_configs) {
    std::string data_path = "/home/vinh/Q32024/CuckooHeavyKeeper/data/CAIDA/only_ip";
    std::ifstream input_file(data_path);
    std::string line;
    std::vector<std::string> data;

    while (getline(input_file, line) && data.size() < app_configs.LINE_READ) { data.push_back(line); }

    run_test<FrequencyEstimatorConfig, std::string>(app_configs, delegation_configs, frequency_estimator_configs, data);
}

template <typename FrequencyEstimatorConfig>
void test_Zipf(ParallelAppConfig &app_configs, DelegationConfig &delegation_configs, FrequencyEstimatorConfig &frequency_estimator_configs) {
    Relation *r1 = generate_relation(app_configs);
    std::vector<unsigned int> data(r1->tuples->begin(), r1->tuples->begin() + app_configs.LINE_READ);

    run_test<FrequencyEstimatorConfig, unsigned int>(app_configs, delegation_configs, frequency_estimator_configs, data);
}

template <typename FrequencyEstimatorConfig> void test(ParallelAppConfig app_configs, DelegationConfig delegation_configs, FrequencyEstimatorConfig frequency_estimator_configs) {
    if (app_configs.DATASET == "CAIDA") {
        test_CAIDA(app_configs, delegation_configs, frequency_estimator_configs);
    } else if (app_configs.DATASET == "zipf") {
        test_Zipf(app_configs, delegation_configs, frequency_estimator_configs);
    } else {
        std::cerr << "Invalid dataset" << std::endl;
    }
}
