#pragma once
#include "delegation_sketch/DelegationConfig.hpp"
#include "delegation_sketch/delegation_sketch_utils.hpp"
#include "frequency_estimator/FrequencyEstimatorTrait.hpp"
#include "frequency_estimator/SequentialHeavyHitterWrapper.hpp"
#include "heavy_hitter_app/AppConfig.hpp"
#include "heavy_hitter_app/Relation.hpp"
#include "heavy_hitter_app/heavy_hitter_test_utils.hpp"

#include "utils/ConfigParser.hpp"
#include "utils/ConfigPrinter.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <string>

template <typename FrequencyEstimator, typename KeyType> void process_data(FrequencyEstimator &frequency_estimator, const std::vector<KeyType> &data, int line_read) {
    for (int i = 0; i < line_read; i++) { frequency_estimator.update(data[i], 1); }
}

template <typename FrequencyEstimator, typename KeyType>
void print_results(FrequencyEstimator &frequency_estimator, const std::map<KeyType, int> &exact_counter, const std::map<KeyType, int> &heavy_hitter_counter,
                   const std::map<KeyType, int> &heavy_hitter_counter_2, int count_correct, double execution_time, int line_read) {
    auto print_section = [](const std::string &title) { std::cout << "# " << title << std::endl; };

    auto print_metric = [](const std::string &name, const auto &value) { std::cout << name << ": " << value << std::endl; };

    size_t exact_count = exact_counter.size();
    size_t hh_count = heavy_hitter_counter.size();
    size_t hh_candidate_count = heavy_hitter_counter_2.size();
    double are_hh = frequency_estimator.ARE(heavy_hitter_counter);
    double aae_hh = frequency_estimator.AAE(heavy_hitter_counter);
    double are_hh_candidates = frequency_estimator.ARE(heavy_hitter_counter_2);
    double aae_hh_candidates = frequency_estimator.AAE(heavy_hitter_counter_2);
    double precision = static_cast<double>(count_correct) / hh_candidate_count;
    double recall = static_cast<double>(count_correct) / hh_count;
    double f1_score = 2 * (precision * recall) / (precision + recall);
    double throughput = line_read / execution_time * 1000;

    print_section("sketch + heap heavy hitters");
    print_metric("Execution Time", execution_time);
    print_metric("Throughput", throughput);
    print_metric("Count distinct", exact_count);
    print_metric("Total heavy hitters", hh_count);
    print_metric("Total heavy_hitter candidates", hh_candidate_count);

    print_section("sample set: true heavy_hitter_counter");
    print_metric("ARE", are_hh);
    print_metric("AAE", aae_hh);
    print_metric("Precision", std::to_string(count_correct) + " over " + std::to_string(hh_candidate_count));
    print_metric("Recall", std::to_string(count_correct) + " over " + std::to_string(hh_count));
    print_metric("F1 Score", f1_score);

    // print compare
    // frequency_estimator.print_compare(heavy_hitter_counter);

    std::cout << "---------------------" << std::endl;

    print_section("sample set: heavy_hitter candidates");
    print_metric("ARE", are_hh_candidates);
    print_metric("AAE", aae_hh_candidates);
    // frequency_estimator.print_compare(heavy_hitter_counter);

    std::cout << "---------------------" << std::endl;

    std::cout << "RESULT_SUMMARY:" << " FrequencyEstimator=" << ConfigPrinter<FrequencyEstimator>::demangle(typeid(FrequencyEstimator).name()) << " TotalHeavyHitters=" << hh_count
              << " TotalHeavyHitterCandidates=" << hh_candidate_count << " Precision=" << precision << " Recall=" << recall << " F1Score=" << f1_score << " ARE=" << are_hh
              << " AAE=" << aae_hh << " ExecutionTime=" << execution_time << " Throughput=" << std::fixed << throughput << std::endl;

    // std::cout << frequency_estimator << std::endl;
}

template <typename FrequencyEstimatorConfig, typename KeyType>
void run_test(FrequencyEstimatorConfig &frequency_estimator_configs, const std::vector<KeyType> &data, SequentialAppConfig &app_configs) {
    using FrequencyEstimator = typename FrequencyEstimatorTrait<FrequencyEstimatorConfig>::type;
    int line_read = app_configs.LINE_READ;
    float theta = app_configs.THETA;
    int num_runs = app_configs.NUM_RUNS;

    std::map<KeyType, int> exact_counter = get_exact_counter<FrequencyEstimator, KeyType>(data, line_read);

    std::map<KeyType, int> heavy_hitter_counter;
    float threshold = theta * line_read;
    for (const auto &entry : exact_counter) {
        if (entry.second >= threshold) { heavy_hitter_counter[entry.first] = entry.second; }
    }

    for (int i = 0; i < num_runs; i++) {
        std::cout << "## Run " << i << std::endl;
        FrequencyEstimator frequency_estimator(frequency_estimator_configs);
        SequentialHeavyHitterWrapper<FrequencyEstimator, KeyType> heavy_hitter_tracker(frequency_estimator, app_configs.THETA);

        auto start = std::chrono::high_resolution_clock::now();

        process_data(heavy_hitter_tracker, data, line_read);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::map<KeyType, int> heavy_hitter_counter_2;
        int count_correct = 0;

        for (const auto &top : heavy_hitter_tracker.get_heavy_hitters()) {
            if (top.second >= threshold) {
                heavy_hitter_counter_2[top.first] = exact_counter[top.first];
                // cout << top.first << " " << top.second << " " << exact_counter[top.first] << endl;
                if (heavy_hitter_counter.count(top.first)) { count_correct += 1; }
            }
        }

        print_results<FrequencyEstimator, KeyType>(frequency_estimator, exact_counter, heavy_hitter_counter, heavy_hitter_counter_2, count_correct, duration.count(), line_read);
    }
}

template <typename FrequencyEstimatorConfig> void test_CAIDA(FrequencyEstimatorConfig &frequency_estimator_configs, SequentialAppConfig &app_configs) {
    std::string data_path = "/home/vinh/Q32024/CuckooHeavyKeeper/data/CAIDA/only_ip";
    std::ifstream input_file(data_path);
    std::string line;
    std::vector<std::string> data;

    while (getline(input_file, line) && data.size() < app_configs.LINE_READ) { data.push_back(line); }

    run_test<FrequencyEstimatorConfig, std::string>(frequency_estimator_configs, data, app_configs);
}

template <typename FrequencyEstimatorConfig> void test_Zipf(FrequencyEstimatorConfig &frequency_estimator_configs, SequentialAppConfig &app_configs) {
    Relation *r1 = generate_relation(app_configs);
    std::vector<unsigned int> data(r1->tuples->begin(), r1->tuples->begin() + app_configs.LINE_READ);
    // std::vector<std::string> data;
    // for (size_t i = 0; i < app_configs.LINE_READ && i < r1->tuples->size(); ++i) { data.push_back(std::to_string((*r1->tuples)[i])); }

    run_test<FrequencyEstimatorConfig, unsigned int>(frequency_estimator_configs, data, app_configs);
    // run_test<FrequencyEstimatorConfig, std::string>(frequency_estimator_configs, data, app_configs);
}

template <typename FrequencyEstimatorConfig> void test(FrequencyEstimatorConfig frequency_estimator_configs, SequentialAppConfig &app_configs) {
    if (app_configs.DATASET == "CAIDA") {
        test_CAIDA(frequency_estimator_configs, app_configs);
    } else if (app_configs.DATASET == "zipf") {
        test_Zipf(frequency_estimator_configs, app_configs);
    } else {
        std::cerr << "Invalid dataset" << std::endl;
    }
}