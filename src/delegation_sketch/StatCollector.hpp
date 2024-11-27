#pragma once

#include "frequency_estimator/MacroPreprocessor.hpp"
#include "json/json.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using std::vector, std::map, std::string, std::cout, std::endl, std::ofstream, std::to_string;

class ThreadPairWiseStatCollector {
  public:
    int count_delegate_to_j_items = 0;
    int count_delegate_to_j_filters = 0;
    int count_delegate_to_j_due_to_filtersize_filters = 0;
    int count_delegate_to_j_due_to_capacity_filters = 0;
    int count_delegate_to_j_blocked = 0;
    int count_delegate_to_j_blocked_loops = 0;
    int count_delegated_from_j_items = 0;
    int count_delegated_from_j_filters = 0;
    int count_use_double_buffering = 0;

    void update_delegate_to_j_items(int count = 1);
    void update_delegate_to_j_filters(int count = 1);
    void update_delegate_to_j_due_to_filtersize_filters(int count = 1);
    void update_delegate_to_j_due_to_capacity_filters(int count = 1);
    void update_delegate_to_j_blocked(int count = 1);
    void update_delegate_to_j_blocked_loops(int count = 1);
    void update_delegated_from_j_items(int count = 1);
    void update_delegated_from_j_filters(int count = 1);
    void update_use_double_buffering(int count = 1);
    map<string, int> get_all_metrics();
    void reset();
};

class ThreadOverallStatCollector {
  public:
    int count_received_from_stream_items;
    int count_query_processed;

    void update_received_from_stream_items(int count = 1);
    void update_query_processed(int count = 1);
    void reset();

    static int calculate_count_delegate_to_threads_items(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_delegate_to_threads_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_delegate_to_threads_due_to_filtersize_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_delegate_to_threads_due_to_capacity_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_delegate_to_threads_blocked(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_delegate_to_threads_blocked_loops(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_delegated_from_threads_items(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_delegated_from_threads_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static int calculate_count_use_double_buffering(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors);
    static map<string, int> calculate_all_metrics(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors, ThreadOverallStatCollector thread_overall_stat_collector);
};

void print_metrics(map<string, int> metrics);

void print_all_threads_metrics_to_json(vector<vector<ThreadPairWiseStatCollector>> all_thread_pairwise_stat_collectors,
                                       vector<ThreadOverallStatCollector> all_thread_overall_stat_collectors, string output_file_path = "");
