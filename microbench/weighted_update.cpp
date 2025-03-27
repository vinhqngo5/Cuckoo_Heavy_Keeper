#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

// Constants
const double b = 1.08;   // decay base
const int Thp = 16;      // promotion threshold

// Precompute de[] array - expected number of decay operations needed to reduce counter from k to 0
std::vector<double> precompute_de() {
    std::vector<double> de(Thp + 1, 0.0);
    for (int k = 1; k <= Thp; ++k) { de[k] = de[k - 1] + std::pow(b, k); }
    return de;
}

// Method 1: Perform repeated unweighted updates (w times)
int compute_repeated_unweighted(int C, int w) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    int current_C = C;
    for (int i = 0; i < w; ++i) {
        if (current_C == 0) break;   // Already reached zero

        // Decay with probability b^(-current_C)
        double decay_prob = std::pow(b, -current_C);
        if (dis(gen) <= decay_prob) { current_C--; }
    }
    return current_C;
}

// Method 2: Compute expected counter value using logarithmic formula
double compute_expected_counter_logarithmic(int C, int w) {
    // E[dc_{C,w}] = log_b(b^C - (w(b-1)/b))
    double result = std::log(std::pow(b, C) - (w * (b - 1.0) / b)) / std::log(b);
    return std::max(0.0, result);   // Ensure result is non-negative
}

// Method 3: Compute expected counter value using precomputed de[] array and binary search
int compute_expected_counter_lookup(int C, int w, const std::vector<double> &de) {
    if (w >= de[C]) {
        // The weight is enough to reduce the counter to 0
        return 0;
    }

    // Binary search to find the expected value after w decay operations
    int left = 0;
    int right = C;
    while (left < right) {
        int mid = left + (right - left) / 2;

        if (de[mid] + w >= de[C]) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return left;   // Return the leftmost position satisfying the condition
}

int main() {
    // Precompute de[] array
    std::vector<double> de = precompute_de();

    // Define constants for the benchmark
    const int NUM_TESTS = 1000000;                           // number of test cases for Method 2 and Method 3
    const int REPEATED_TESTS = std::min(10000, NUM_TESTS);   // fewer tests for "slow" Method 1

    // Generate random counter values and weights
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> counter_dist(1, Thp);
    std::uniform_int_distribution<> weight_dist(1, static_cast<int>(de[Thp]));

    std::vector<int> counters(NUM_TESTS);
    std::vector<int> weights(NUM_TESTS);

    for (int i = 0; i < NUM_TESTS; ++i) {
        counters[i] = counter_dist(gen);
        weights[i] = weight_dist(gen);
    }

    // Benchmark Method 1: Repeated unweighted updates
    // Using a smaller subset for this method as it's much slower
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<int> results_repeated(REPEATED_TESTS);

    for (int i = 0; i < REPEATED_TESTS; ++i) { results_repeated[i] = compute_repeated_unweighted(counters[i], weights[i]); }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto repeated_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    // Scale up to estimate full run time
    double full_repeated_duration = static_cast<double>(repeated_duration) * (NUM_TESTS / REPEATED_TESTS);

    // Benchmark Method 2: Logarithmic formula
    start_time = std::chrono::high_resolution_clock::now();
    std::vector<double> results_log(NUM_TESTS);

    for (int i = 0; i < NUM_TESTS; ++i) { results_log[i] = compute_expected_counter_logarithmic(counters[i], weights[i]); }

    end_time = std::chrono::high_resolution_clock::now();
    auto log_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Benchmark Method 3: Lookup table with binary search
    start_time = std::chrono::high_resolution_clock::now();
    std::vector<int> results_lookup(NUM_TESTS);

    for (int i = 0; i < NUM_TESTS; ++i) { results_lookup[i] = compute_expected_counter_lookup(counters[i], weights[i], de); }

    end_time = std::chrono::high_resolution_clock::now();
    auto lookup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Calculate statistics for comparisons between methods
    int match_count_12 = 0;
    double max_diff_12 = 0.0;
    double avg_diff_12 = 0.0;

    int match_count_13 = 0;
    double max_diff_13 = 0.0;
    double avg_diff_13 = 0.0;

    int match_count_23 = 0;
    double max_diff_23 = 0.0;
    double avg_diff_23 = 0.0;

    // Compare Method 1 and Method 2 (for the reduced set)
    for (int i = 0; i < REPEATED_TESTS; ++i) {
        double diff = std::abs(results_repeated[i] - results_log[i]);
        max_diff_12 = std::max(max_diff_12, diff);
        avg_diff_12 += diff;
        if (diff < 1.0) match_count_12++;
    }
    avg_diff_12 /= REPEATED_TESTS;

    // Compare Method 1 and Method 3 (for the reduced set)
    for (int i = 0; i < REPEATED_TESTS; ++i) {
        double diff = std::abs(results_repeated[i] - results_lookup[i]);
        max_diff_13 = std::max(max_diff_13, diff);
        avg_diff_13 += diff;
        if (diff < 1.0) match_count_13++;
    }
    avg_diff_13 /= REPEATED_TESTS;

    // Compare Method 2 and Method 3 (for the full set)
    for (int i = 0; i < NUM_TESTS; ++i) {
        double diff = std::abs(results_log[i] - results_lookup[i]);
        max_diff_23 = std::max(max_diff_23, diff);
        avg_diff_23 += diff;
        if (diff < 1.0) match_count_23++;
    }
    avg_diff_23 /= NUM_TESTS;

    // Print results
    std::cout << "Benchmark Results:" << std::endl;
    std::cout << "----------------" << std::endl;
    std::cout << "Decay base (b): " << b << std::endl;
    std::cout << "Threshold (Thp): " << Thp << std::endl;
    std::cout << "Number of test cases: " << NUM_TESTS << " (Method 1: " << REPEATED_TESTS << ")" << std::endl;

    // Calculate per-operation timing in nanoseconds
    double method1_time_per_op_ns = (repeated_duration * 1000000.0) / REPEATED_TESTS;
    double method2_time_per_op_ns = (log_duration * 1000000.0) / NUM_TESTS;
    double method3_time_per_op_ns = (lookup_duration * 1000000.0) / NUM_TESTS;

    // Per-operation performance section
    std::cout << "Per-Operation Performance:" << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "Method 1 (Repeated unweighted): " << std::fixed << std::setprecision(1) << method1_time_per_op_ns << " ns (" << method1_time_per_op_ns / 1000 << " µs) ["
              << REPEATED_TESTS << " repetitions]" << std::endl;

    std::cout << "Method 2 (Logarithmic formula): " << std::fixed << std::setprecision(1) << method2_time_per_op_ns << " ns [" << NUM_TESTS << " repetitions]" << std::endl;

    std::cout << "Method 3 (Lookup table): " << std::fixed << std::setprecision(1) << method3_time_per_op_ns << " ns [" << NUM_TESTS << " repetitions]" << std::endl;
    std::cout << std::endl;

    // Total execution times
    std::cout << "Total Execution Times:" << std::endl;
    std::cout << "--------------------" << std::endl;
    std::cout << "Method 1 - Repeated unweighted updates: " << repeated_duration << " ms (" << REPEATED_TESTS << " cases)" << std::endl;
    std::cout << "Method 1 - Estimated for all " << NUM_TESTS << " cases: " << std::fixed << std::setprecision(1) << full_repeated_duration << " ms" << std::endl;
    std::cout << "Method 2 - Logarithmic formula: " << log_duration << " ms" << std::endl;
    std::cout << "Method 3 - Lookup table: " << lookup_duration << " ms" << std::endl;
    std::cout << std::endl;

    std::cout << "Performance Comparisons:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << "Method 1 vs Method 2 speedup: " << std::fixed << std::setprecision(2) << full_repeated_duration / log_duration << "x" << std::endl;
    std::cout << "Method 1 vs Method 3 speedup: " << std::fixed << std::setprecision(2) << full_repeated_duration / lookup_duration << "x" << std::endl;
    std::cout << "Method 2 vs Method 3 speedup: " << std::fixed << std::setprecision(2) << static_cast<double>(log_duration) / lookup_duration << "x" << std::endl;
    std::cout << std::endl;

    std::cout << "Correctness Verification:" << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "Method 1 vs Method 2 - Match percentage: " << std::fixed << std::setprecision(2) << (static_cast<double>(match_count_12) / REPEATED_TESTS) * 100 << "%"
              << std::endl;
    std::cout << "Method 1 vs Method 2 - Average difference: " << avg_diff_12 << std::endl;
    std::cout << "Method 1 vs Method 2 - Maximum difference: " << max_diff_12 << std::endl;
    std::cout << std::endl;

    std::cout << "Method 1 vs Method 3 - Match percentage: " << std::fixed << std::setprecision(2) << (static_cast<double>(match_count_13) / REPEATED_TESTS) * 100 << "%"
              << std::endl;
    std::cout << "Method 1 vs Method 3 - Average difference: " << avg_diff_13 << std::endl;
    std::cout << "Method 1 vs Method 3 - Maximum difference: " << max_diff_13 << std::endl;
    std::cout << std::endl;

    std::cout << "Method 2 vs Method 3 - Match percentage: " << std::fixed << std::setprecision(2) << (static_cast<double>(match_count_23) / NUM_TESTS) * 100 << "%" << std::endl;
    std::cout << "Method 2 vs Method 3 - Average difference: " << avg_diff_23 << std::endl;
    std::cout << "Method 2 vs Method 3 - Maximum difference: " << max_diff_23 << std::endl;

    // Print sample comparisons
    std::cout << std::endl << "Sample comparisons (first 5 cases):" << std::endl;
    std::cout << "Counter\tWeight\tMethod 1\tMethod 2\tMethod 3" << std::endl;
    for (int i = 0; i < std::min(5, REPEATED_TESTS); ++i) {
        std::cout << counters[i] << "\t" << weights[i] << "\t" << results_repeated[i] << "\t\t" << std::fixed << std::setprecision(2) << results_log[i] << "\t\t"
                  << results_lookup[i] << std::endl;
    }

    return 0;
}