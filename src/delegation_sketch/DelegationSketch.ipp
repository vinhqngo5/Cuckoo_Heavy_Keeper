// DelegationSketch implementation
template <typename FrequencyEstimator>
DelegationSketch<FrequencyEstimator>::DelegationSketch(ParallelAppConfig app_configs, DelegationConfig delegation_configs, std::vector<FrequencyEstimator> &frequency_estimators) {
    this->num_threads = app_configs.NUM_THREADS;
    this->filter_size = delegation_configs.FILTER_SIZE;
    this->delegation_configs = delegation_configs;
    for (int i = 0; i < num_threads; ++i) {
        thread_local_delegation_sketches.push_back(
            new ThreadLocalDelegationSketch<FrequencyEstimator>(app_configs, delegation_configs, i, std::move(frequency_estimators[i]), this));
    }

    precompute_mods(app_configs.NUM_THREADS);
}

template <typename FrequencyEstimator> int DelegationSketch<FrequencyEstimator>::direct_query(const int &key) {
    int owner = find_owner(key);
    return this->thread_local_delegation_sketches[owner]->frequency_estimator.estimate(key);
}

template <typename FrequencyEstimator> template <typename T> float DelegationSketch<FrequencyEstimator>::ARE(const std::map<T, int> &exact_counter) {
    float relative_error = 0;
    for (const auto &entry : exact_counter) { relative_error += float(abs(entry.second - (int) this->direct_query(entry.first))) / entry.second; }
    return relative_error / exact_counter.size();
}

template <typename FrequencyEstimator> template <typename T> float DelegationSketch<FrequencyEstimator>::AAE(const std::map<T, int> &exact_counter) {
    float absolute_error = 0;
    for (const auto &entry : exact_counter) { absolute_error += float(abs(entry.second - (int) this->direct_query(entry.first))); }
    return absolute_error / exact_counter.size();
}

// ThreadLocalDelegationSketch implementation
template <typename FrequencyEstimator>
ThreadLocalDelegationSketch<FrequencyEstimator>::ThreadLocalDelegationSketch(ParallelAppConfig app_configs, DelegationConfig delegation_configs, int current_thread_id,
                                                                             FrequencyEstimator frequency_estimator, DelegationSketch<FrequencyEstimator> *delegation_sketch) {
    this->current_thread_id = current_thread_id;
    this->frequency_estimator = frequency_estimator;
    this->delegation_configs = delegation_configs;
    this->FILTER_SIZE = delegation_configs.FILTER_SIZE;
    this->delegation_sketch = delegation_sketch;
    this->delegation_filters = std::vector<DelegationFilter *>();
    this->pending_queries = std::vector<PendingQuery *>();

    this->thread_pairwise_stat_collectors = std::vector<ThreadPairWiseStatCollector>(delegation_sketch->num_threads);
    this->thread_overall_stat_collector = ThreadOverallStatCollector();

    this->seeds = seed_rand();
    for (int i = 0; i < delegation_sketch->num_threads; ++i) {
        this->double_buffer_delegation_filters[0].push_back(std::move(new DelegationFilter(FILTER_SIZE)));
        this->double_buffer_delegation_filters[1].push_back(std::move(new DelegationFilter(FILTER_SIZE)));
        current_buffer_ids.push_back(0);
        this->delegation_filters.push_back((DelegationFilter *) this->double_buffer_delegation_filters[0][i]);
        this->pending_queries.push_back(std::move(new PendingQuery()));
    }
}

template <typename FrequencyEstimator> void ThreadLocalDelegationSketch<FrequencyEstimator>::process_pending_inserts() {
    if (full_delegate_filters.is_empty()) { return; }

    while (!full_delegate_filters.is_empty()) {
        DelegationFilter *filter;
        full_delegate_filters.pop(filter);

        int filter_size = filter->size.load(std::memory_order_relaxed);
        for (int j = 0; j < filter_size; ++j) {
            int count = this->frequency_estimator.update_and_estimate(filter->keys[j], filter->counts[j]);

            filter->counts[j] = 0;
            filter->keys[j] = 0;
        }

        filter->size = 0;
        filter->lock.store(false, std::memory_order_relaxed);
    }
}

template <typename FrequencyEstimator> void ThreadLocalDelegationSketch<FrequencyEstimator>::flush_pending_inserts() {
    for (int i = 0; i < this->delegation_sketch->num_threads; ++i) {
        auto filter = this->delegation_sketch->thread_local_delegation_sketches[i]->delegation_filters[current_thread_id];
        for (int j = 0; j < filter->size; ++j) {
            this->frequency_estimator.update(filter->keys[j], filter->counts[j]);
            filter->counts[j] = 0;
            filter->keys[j] = 0;
        }
        filter->size = 0;
    }
}

template <typename FrequencyEstimator> void ThreadLocalDelegationSketch<FrequencyEstimator>::insert(const int &key) {
    int owner_thread_id = find_owner(key);

    this->thread_pairwise_stat_collectors[owner_thread_id].update_delegate_to_j_items();

    if (owner_thread_id == current_thread_id) {
        this->insert_directly(key);
        return;
    }

    auto filter = delegation_filters[owner_thread_id];
    bool flag = false;
    if (filter->lock.load(std::memory_order_relaxed) == true) {
        current_buffer_ids[owner_thread_id] = 1 - current_buffer_ids[owner_thread_id];
        delegation_filters[owner_thread_id] = double_buffer_delegation_filters[current_buffer_ids[owner_thread_id]][owner_thread_id];
        filter = delegation_filters[owner_thread_id];
        flag = true;

        this->thread_pairwise_stat_collectors[owner_thread_id].update_use_double_buffering();
    }

    while (filter->lock.load(std::memory_order_relaxed) == true && START_BENCHMARK) {
        process_pending_inserts();
        process_pending_queries();
        if (flag) {
            this->thread_pairwise_stat_collectors[owner_thread_id].update_delegate_to_j_blocked();
            flag = false;
        }
        this->thread_pairwise_stat_collectors[owner_thread_id].update_delegate_to_j_blocked_loops();
    }

    int count = filter->update_or_insert_if_not_full_simd(key);

    int filter_capacity = 1000;
    if (filter->size.load(std::memory_order_relaxed) == FILTER_SIZE) {
        auto owner_sketch = this->delegation_sketch->thread_local_delegation_sketches[owner_thread_id];
        filter->lock.store(true, std::memory_order_relaxed);
        owner_sketch->full_delegate_filters.push(filter);

        this->thread_pairwise_stat_collectors[owner_thread_id].update_delegate_to_j_filters();
        if (count == filter_capacity) {
            this->thread_pairwise_stat_collectors[owner_thread_id].update_delegate_to_j_due_to_capacity_filters();
        } else {
            this->thread_pairwise_stat_collectors[owner_thread_id].update_delegate_to_j_due_to_filtersize_filters();
        }
    }
}

template <typename FrequencyEstimator> void ThreadLocalDelegationSketch<FrequencyEstimator>::insert_directly(const int &key) { frequency_estimator.update(key); }

template <typename FrequencyEstimator> void ThreadLocalDelegationSketch<FrequencyEstimator>::process_pending_queries() {
    return;
    for (int i = 0; i < this->delegation_sketch->num_threads; ++i) {
        auto query = this->pending_queries[i];
        if (query->flag) {
            int count = 0;
            for (int j = 0; j < this->delegation_sketch->num_threads; ++j) {
                auto &filter = this->delegation_sketch->thread_local_delegation_sketches[j]->delegation_filters[current_thread_id];
                count += filter->lookup_value_simd(query->key);
            }
            count += frequency_estimator.estimate(query->key);
            query->count = count;
            query->flag = false;

            for (int j = i + 1; j < this->delegation_sketch->num_threads; ++j) {
                auto next_query = this->pending_queries[j];
                if (next_query->flag && next_query->key == query->key) {
                    next_query->count = count;
                    next_query->flag = false;
                }
            }
        }
    }
}

template <typename FrequencyEstimator> int ThreadLocalDelegationSketch<FrequencyEstimator>::query(const int &key) {
    int owner_thread_id = find_owner(key);
    if (owner_thread_id == current_thread_id) { return this->query_directly(key); }
    auto query = this->delegation_sketch->thread_local_delegation_sketches[owner_thread_id]->pending_queries[current_thread_id];
    query->add_query(key);
    while (query->flag && START_BENCHMARK.load(std::memory_order_relaxed)) {
        process_pending_inserts();
        if (!delegation_configs.QUERY_RATE == 0) { process_pending_queries(); }
    }
    return query->count;
}

template <typename FrequencyEstimator> int ThreadLocalDelegationSketch<FrequencyEstimator>::query_directly(const int &key) {
    int count = 0;
    for (int i = 0; i < this->delegation_sketch->num_threads; ++i) {
        auto &filter = this->delegation_sketch->thread_local_delegation_sketches[i]->delegation_filters[current_thread_id];
        count += filter->lookup_value_simd(key);
    }
    count += frequency_estimator.estimate(key);

    for (int j = 0; j < this->delegation_sketch->num_threads; ++j) {
        if (j == current_thread_id) { continue; }
        auto next_query = this->pending_queries[j];
        if (next_query->flag && next_query->key == key) {
            next_query->count = count;
            next_query->flag = false;
        }
    }
    return count;
}

template <typename FrequencyEstimator>
void start_thread(DelegationConfig delegation_configs, ThreadLocalDelegationSketch<FrequencyEstimator> *thread_local_delegation_sketch, int start, int end, Relation *r1,
                  std::barrier<> &sync_point, atomic<bool> &START_BENCHMARK) {
    setaffinity_oncpu(thread_local_delegation_sketch->current_thread_id + 2);
    sync_point.arrive_and_wait();
    thread_local_delegation_sketch->START_BENCHMARK = &START_BENCHMARK;
    while (START_BENCHMARK.load(std::memory_order_relaxed)) {
        for (int i = start; i < end; i++) {
            if (should_perform_query(thread_local_delegation_sketch->seeds, delegation_configs.QUERY_RATE)) {
                unsigned int key = r1->tuples->at(i);
                int freq = thread_local_delegation_sketch->query(key);
                thread_local_delegation_sketch->query_processed_during_time_interval++;
            }
            unsigned int key = r1->tuples->at(i);
            thread_local_delegation_sketch->insert(key);

            thread_local_delegation_sketch->thread_overall_stat_collector.update_received_from_stream_items();

            thread_local_delegation_sketch->process_pending_inserts();
            thread_local_delegation_sketch->element_processed_during_time_interval++;
            if (!START_BENCHMARK.load(std::memory_order_relaxed)) { break; }
        }
    }
}

template <typename FrequencyEstimator>
DelegationSketch<FrequencyEstimator> *start_threads(AppConfig app_configs, DelegationBuildConfig delegation_build_configs, DelegationConfig delegation_configs, Relation *r1,
                                                    vector<FrequencyEstimator> &frequency_estimators, atomic<bool> &START_BENCHMARK) {
    vector<thread> threads;

    int num_threads = app_configs.NUM_THREADS;
    int filter_size = delegation_configs.FILTER_SIZE;
    int DURATION = app_configs.DURATION;
    int tuples_no = app_configs.tuples_no;
    std::barrier sync_point(num_threads + 1);

    DelegationSketch<FrequencyEstimator> *delegation_sketch = new DelegationSketch<FrequencyEstimator>(app_configs, delegation_configs, frequency_estimators);
    for (int i = 0; i < num_threads; i++) {
        int start = i * (tuples_no / num_threads);
        int end = i == num_threads - 1 ? tuples_no : (i + 1) * (tuples_no / num_threads);
        cout << "thread: " << i << " start: " << start << " end: " << end << " end-start:" << end - start << endl;
        threads.push_back(std::move(std::thread(start_thread<FrequencyEstimator>, delegation_configs, delegation_sketch->thread_local_delegation_sketches[i], start, end, r1,
                                                std::ref(sync_point), std::ref(START_BENCHMARK))));
    }

    sync_point.arrive_and_wait();

    start_time();

    if (DURATION > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(DURATION));
        START_BENCHMARK.store(false, std::memory_order_relaxed);
        stop_time();
    }

    for (int i = 0; i < num_threads; i++) { threads[i].join(); }

    for (int i = 0; i < num_threads; i++) { delegation_sketch->thread_local_delegation_sketches[i]->flush_pending_inserts(); }
    return delegation_sketch;
}

template <typename FrequencyEstimator> void print_stats_for_delegation_sketch(AppConfig app_configs, DelegationSketch<FrequencyEstimator> *delegation_sketch) {
    int num_threads = app_configs.NUM_THREADS;

    // print stats
    long long total_insert_processed = 0, total_query_processed = 0, total_insert_processed_from_sketch = 0;
    long long total_need_to_wait = 0, total_no_need_to_wait = 0, total_count_looping_when_waiting = 0;

    vector<vector<ThreadPairWiseStatCollector>> all_thread_pairwise_stat_collectors;
    vector<ThreadOverallStatCollector> all_thread_overall_stat_collectors;

    for (int i = 0; i < num_threads; i++) {
        total_insert_processed += delegation_sketch->thread_local_delegation_sketches[i]->element_processed_during_time_interval;
        total_insert_processed_from_sketch += delegation_sketch->thread_local_delegation_sketches[i]->frequency_estimator.total;
        total_query_processed += delegation_sketch->thread_local_delegation_sketches[i]->query_processed_during_time_interval;
        cout << "thread " << i << " proccessed:" << delegation_sketch->thread_local_delegation_sketches[i]->element_processed_during_time_interval << endl;
        cout << "thread sketch " << i << " proccessed:" << delegation_sketch->thread_local_delegation_sketches[i]->frequency_estimator.total << endl;
        // update update_delegated_from_j_items
        for (int j = 0; j < num_threads; j++) {
            if (j == i) continue;
            delegation_sketch->thread_local_delegation_sketches[i]->thread_pairwise_stat_collectors[j].update_delegated_from_j_items(
                delegation_sketch->thread_local_delegation_sketches[j]->thread_pairwise_stat_collectors[i].count_delegate_to_j_items);
            delegation_sketch->thread_local_delegation_sketches[i]->thread_pairwise_stat_collectors[j].update_delegated_from_j_filters(
                delegation_sketch->thread_local_delegation_sketches[j]->thread_pairwise_stat_collectors[i].count_delegate_to_j_filters);
        }
        map<string, int> metrics = delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector.calculate_all_metrics(
            delegation_sketch->thread_local_delegation_sketches[i]->thread_pairwise_stat_collectors,
            delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector);
        print_metrics(metrics);
        cout << "----" << endl;
        all_thread_pairwise_stat_collectors.push_back(delegation_sketch->thread_local_delegation_sketches[i]->thread_pairwise_stat_collectors);
        all_thread_overall_stat_collectors.push_back(delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector);
    }

    print_all_threads_metrics_to_json(all_thread_pairwise_stat_collectors, all_thread_overall_stat_collectors);

    // notes: total insert processed might be slightly different from total insert processed from
    // sketch due to when stop the benchmark, the current insert might not be finished
    cout << "num_threads: " << app_configs.NUM_THREADS << " total insert processed: " << float(total_insert_processed) / 1000000 << "Mops time process: " << get_time_ms() / 1000
         << endl;
    cout << "num_threads: " << app_configs.NUM_THREADS << " total insert processed from sketch: " << float(total_insert_processed_from_sketch) / 1000000
         << "Mops time process: " << get_time_ms() / 1000 << endl;
    cout << "num_threads: " << app_configs.NUM_THREADS << " total query processed: " << float(total_query_processed) / 1000000 << "Mops time process: " << get_time_ms() / 1000
         << endl;
    cout << "Throughput: " << float(total_insert_processed) / 1000000 << endl;
}