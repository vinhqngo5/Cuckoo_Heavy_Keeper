#include "StatCollector.hpp"

void ThreadPairWiseStatCollector::update_delegate_to_j_items(int count) { count_delegate_to_j_items += count; }

void ThreadPairWiseStatCollector::update_delegate_to_j_filters(int count) { count_delegate_to_j_filters += count; }

void ThreadPairWiseStatCollector::update_delegate_to_j_due_to_filtersize_filters(int count) { count_delegate_to_j_due_to_filtersize_filters += count; }

void ThreadPairWiseStatCollector::update_delegate_to_j_due_to_capacity_filters(int count) { count_delegate_to_j_due_to_capacity_filters += count; }

void ThreadPairWiseStatCollector::update_delegate_to_j_blocked(int count) { count_delegate_to_j_blocked += count; }

void ThreadPairWiseStatCollector::update_delegate_to_j_blocked_loops(int count) { count_delegate_to_j_blocked_loops += count; }

void ThreadPairWiseStatCollector::update_delegated_from_j_items(int count) { count_delegated_from_j_items += count; }

void ThreadPairWiseStatCollector::update_delegated_from_j_filters(int count) { count_delegated_from_j_filters += count; }

void ThreadPairWiseStatCollector::update_use_double_buffering(int count) { count_use_double_buffering += count; }

map<string, int> ThreadPairWiseStatCollector::get_all_metrics() {
    map<string, int> metrics;
    metrics["count_delegate_to_j_items"] = count_delegate_to_j_items;
    metrics["count_delegate_to_j_filters"] = count_delegate_to_j_filters;
    metrics["count_delegate_to_j_due_to_filtersize_filters"] = count_delegate_to_j_due_to_filtersize_filters;
    metrics["count_delegate_to_j_due_to_capacity_filters"] = count_delegate_to_j_due_to_capacity_filters;
    metrics["count_delegate_to_j_blocked"] = count_delegate_to_j_blocked;
    metrics["count_delegate_to_j_blocked_loops"] = count_delegate_to_j_blocked_loops;
    metrics["count_delegated_from_j_items"] = count_delegated_from_j_items;
    metrics["count_delegated_from_j_filters"] = count_delegated_from_j_filters;
    metrics["count_use_double_buffering"] = count_use_double_buffering;
    return metrics;
}

void ThreadPairWiseStatCollector::reset() {
    count_delegate_to_j_items = 0;
    count_delegate_to_j_filters = 0;
    count_delegate_to_j_due_to_filtersize_filters = 0;
    count_delegate_to_j_due_to_capacity_filters = 0;
    count_delegate_to_j_blocked = 0;
    count_delegate_to_j_blocked_loops = 0;
    count_delegated_from_j_items = 0;
    count_delegated_from_j_filters = 0;
}

void ThreadOverallStatCollector::update_received_from_stream_items(int count) { count_received_from_stream_items += count; }

void ThreadOverallStatCollector::update_query_processed(int count) { count_query_processed += count; }

void ThreadOverallStatCollector::reset() { count_received_from_stream_items = 0; }

int ThreadOverallStatCollector::calculate_count_delegate_to_threads_items(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (int i = 0; i < thread_pairwise_stat_collectors.size(); i++) { count += thread_pairwise_stat_collectors[i].count_delegate_to_j_items; }
    return count;
}

int ThreadOverallStatCollector::calculate_count_delegate_to_threads_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) { count += thread_pairwise_stat_collector.count_delegate_to_j_filters; }
    return count;
}

int ThreadOverallStatCollector::calculate_count_delegate_to_threads_due_to_filtersize_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) {
        count += thread_pairwise_stat_collector.count_delegate_to_j_due_to_filtersize_filters;
    }
    return count;
}

int ThreadOverallStatCollector::calculate_count_delegate_to_threads_due_to_capacity_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) {
        count += thread_pairwise_stat_collector.count_delegate_to_j_due_to_capacity_filters;
    }
    return count;
}

int ThreadOverallStatCollector::calculate_count_delegate_to_threads_blocked(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) { count += thread_pairwise_stat_collector.count_delegate_to_j_blocked; }
    return count;
}

int ThreadOverallStatCollector::calculate_count_delegate_to_threads_blocked_loops(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) {
        count += thread_pairwise_stat_collector.count_delegate_to_j_blocked_loops;
    }
    return count;
}

int ThreadOverallStatCollector::calculate_count_delegated_from_threads_items(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) { count += thread_pairwise_stat_collector.count_delegated_from_j_items; }
    return count;
}

int ThreadOverallStatCollector::calculate_count_delegated_from_threads_filters(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) { count += thread_pairwise_stat_collector.count_delegated_from_j_filters; }
    return count;
}

int ThreadOverallStatCollector::calculate_count_use_double_buffering(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors) {
    int count = 0;
    for (ThreadPairWiseStatCollector thread_pairwise_stat_collector : thread_pairwise_stat_collectors) { count += thread_pairwise_stat_collector.count_use_double_buffering; }
    return count;
}

map<string, int> ThreadOverallStatCollector::calculate_all_metrics(vector<ThreadPairWiseStatCollector> thread_pairwise_stat_collectors,
                                                                   ThreadOverallStatCollector thread_overall_stat_collector) {
    map<string, int> metrics;
    metrics["count_delegate_to_threads_items"] = calculate_count_delegate_to_threads_items(thread_pairwise_stat_collectors);
    metrics["count_delegate_to_threads_filters"] = calculate_count_delegate_to_threads_filters(thread_pairwise_stat_collectors);
    metrics["count_delegate_to_threads_due_to_filtersize_filters"] = calculate_count_delegate_to_threads_due_to_filtersize_filters(thread_pairwise_stat_collectors);
    metrics["count_delegate_to_threads_due_to_capacity_filters"] = calculate_count_delegate_to_threads_due_to_capacity_filters(thread_pairwise_stat_collectors);
    metrics["count_delegate_to_threads_blocked"] = calculate_count_delegate_to_threads_blocked(thread_pairwise_stat_collectors);
    metrics["count_delegate_to_threads_blocked_loops"] = calculate_count_delegate_to_threads_blocked_loops(thread_pairwise_stat_collectors);
    metrics["count_delegated_from_threads_items"] = calculate_count_delegated_from_threads_items(thread_pairwise_stat_collectors);
    metrics["count_delegated_from_threads_filters"] = calculate_count_delegated_from_threads_filters(thread_pairwise_stat_collectors);
    metrics["count_use_double_buffering"] = calculate_count_use_double_buffering(thread_pairwise_stat_collectors);
    metrics["count_received_from_stream_items"] = thread_overall_stat_collector.count_received_from_stream_items;
    return metrics;
}

void print_metrics(map<string, int> metrics) {
    for (auto metric : metrics) { cout << metric.first << " : " << metric.second << endl; }
}

void print_all_threads_metrics_to_json(vector<vector<ThreadPairWiseStatCollector>> all_thread_pairwise_stat_collectors,
                                       vector<ThreadOverallStatCollector> all_thread_overall_stat_collectors, string output_file_path) {

    if (output_file_path.empty()) {
        cout << "output_file_path not found, not printing metrics to file " << endl;
    } else {
        cout << "Printing metrics to file: " << output_file_path << endl;
    }

    using json = nlohmann::json;
    json result;
    auto now = std::chrono::system_clock::now();
    auto time_start = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_start), "+%F_%H%M%S");
    result["time_start"] = ss.str();

    for (int i = 0; i < all_thread_pairwise_stat_collectors.size(); i++) {
        // create nested json object for each thread
        json thread_metrics;
        for (int j = 0; j < all_thread_pairwise_stat_collectors[i].size(); j++) {
            thread_metrics["thread_" + to_string(j)] = all_thread_pairwise_stat_collectors[i][j].get_all_metrics();
        }
        thread_metrics["overall"] = all_thread_overall_stat_collectors[i].calculate_all_metrics(all_thread_pairwise_stat_collectors[i], all_thread_overall_stat_collectors[i]);
        result["thread_" + to_string(i)] = thread_metrics;
    }

    ofstream outputFile(output_file_path);
    outputFile << result.dump(4) << endl;
    outputFile.close();
}
