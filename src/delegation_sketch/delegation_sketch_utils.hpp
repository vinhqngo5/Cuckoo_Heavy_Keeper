#pragma once

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <sched.h>
#include <sys/time.h>
#include <vector>

#include "heavy_hitter_app/AppConfig.hpp"
#include "utils/getticks.hpp"

#define delegation_query_random xorshf96

using std::vector, std::default_random_engine, std::begin, std::shuffle;

unsigned long *seed_rand();
unsigned long xorshf96(unsigned long *x, unsigned long *y, unsigned long *z);
bool should_perform_query(unsigned long *seeds, double query_rate);

void start_time();
void stop_time();
unsigned long get_time_ms();

void setaffinity_oncpu(unsigned int cpu);

extern unsigned short precomputed_mods[512];

inline int find_owner(unsigned int key) { return precomputed_mods[key & 511]; }

void precompute_mods(int num_threads);

template <typename T> float ARE(const std::map<T, int> &exact_counter, const std::map<T, int> &approx_counter) {
    float relative_error = 0;
    for (const auto &entry : exact_counter) {
        auto it = approx_counter.find(entry.first);
        if (it != approx_counter.end()) {
            relative_error += float(abs(entry.second - it->second) / entry.second);
        } else {
            relative_error += 1.0f;
        }
    }
    return relative_error / exact_counter.size();
}

template <typename T> float AAE(const std::map<T, int> &exact_counter, const std::map<T, int> &approx_counter) {
    float absolute_error = 0;
    for (const auto &entry : exact_counter) {
        auto it = approx_counter.find(entry.first);
        if (it != approx_counter.end()) {
            absolute_error += abs(entry.second - it->second);
        } else {
            absolute_error += entry.second;
        }
    }
    return absolute_error / exact_counter.size();
}

template <typename T> void print_compare(const std::map<T, int> &exact_counter, const std::map<T, int> &approx_counter, string output_file_path = "") {
    std::ofstream output_file;
    if (!output_file_path.empty()) {
        output_file.open(output_file_path);
        if (!output_file.is_open()) {
            std::cerr << "Failed to open output file: " << output_file_path << std::endl;
            // Continue execution, but only output to cout
        }
    }

    // Function to print to both cout and file if specified
    auto print = [&output_file](const auto &x) {
        std::cout << x;
        if (output_file.is_open()) output_file << x;
    };

    for (const auto &entry : exact_counter) {
        auto it = approx_counter.find(entry.first);
        if (it != approx_counter.end()) {
            print("Key: " + std::to_string(entry.first) + " Exact: " + std::to_string(entry.second) + " Approx: " + std::to_string(it->second) + "\n");
        } else {
            print("Key: " + std::to_string(entry.first) + " Exact: " + std::to_string(entry.second) + " Approx: 0\n");
        }
    }
}