
bool LocalHeavyHitterTracker::add_if_is_local_heavy_hitter(int key, int difference, int count) {
    if (count < threshold) { return false; }

    local_heavy_hitter_differences.push_back(make_tuple(key, difference, count));

    if (local_heavy_hitters.contains(key)) {
        local_heavy_hitters.update_add(key, difference);
        return true;
    } else {
        local_heavy_hitters.push(key, count);
        return true;
    }
    return false;
}

void LocalHeavyHitterTracker::update_threshold(int threshold) { this->threshold = threshold; }

void LocalHeavyHitterTracker::update_global_heavy_hitters(GlobalHeavyHitterTracker &global_heavy_hitter_tracker, int total_differences) {
    for (auto &el : local_heavy_hitter_differences) {
        int key = get<0>(el);
        int difference = get<1>(el);
        int count = get<2>(el);
        global_heavy_hitter_tracker.global_heavy_hitters.upsert(key, [difference](int &num) { num += difference; }, count);
    }

    global_heavy_hitter_tracker.stream_size.fetch_add(total_differences);

    if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy") {
        if (global_heavy_hitter_tracker.stream_size.load() >= DelegationBuildConfig::evaluate_accuracy_stream_size) {
            if constexpr (DelegationBuildConfig::evaluate_accuracy_when == "start") { delegation_sketch_context.START_BENCHMARK.store(false, std::memory_order_relaxed); }
            delegation_sketch_context.START_ACCURACY_EVALUATION.store(true, std::memory_order_relaxed);
        }
    } else if constexpr (DelegationBuildConfig::evaluate_mode == "latency") {
        if (global_heavy_hitter_tracker.stream_size.load() >= DelegationBuildConfig::evaluate_accuracy_stream_size) {
            delegation_sketch_context.START_ACCURACY_EVALUATION.store(true, std::memory_order_relaxed);
        }
    }

    this->update_threshold(global_heavy_hitter_tracker.stream_size.load() * delegation_sketch_context.app_configs.THETA);

    auto popped_items = local_heavy_hitters.pop_all_below(threshold);
    for (auto &el : popped_items) { global_heavy_hitter_tracker.global_heavy_hitters.erase(el.first); }

    local_heavy_hitter_differences.clear();
}

// DelegationHeavyHitter implementation
template <typename FrequencyEstimator>
DelegationHeavyHitter<FrequencyEstimator>::DelegationHeavyHitter(DelegationSketchContext &delegation_sketch_context, std::vector<FrequencyEstimator> &frequency_estimators)
    : delegation_sketch_context(delegation_sketch_context), global_heavy_hitter_tracker(delegation_sketch_context) {
    int num_threads = delegation_sketch_context.app_configs.NUM_THREADS;

    for (int i = 0; i < num_threads; ++i) {
        thread_local_delegation_sketches.push_back(new ThreadLocalDelegationHeavyHitter<FrequencyEstimator>(std::ref(delegation_sketch_context), i, frequency_estimators[i], this));
    }

    precompute_mods(num_threads);
    global_heavy_hitter_tracker.global_heavy_hitters = libcuckoo::cuckoohash_map<int, int>(2048);
}

template <typename FrequencyEstimator> int DelegationHeavyHitter<FrequencyEstimator>::direct_query(const int &key) {
    int owner = find_owner(key);
    return this->thread_local_delegation_sketches[owner]->frequency_estimator.estimate(key);
}

template <typename FrequencyEstimator> void DelegationHeavyHitter<FrequencyEstimator>::query_all_heavy_hitters(map<int, int> &result) {
#if EQUAL(PARALLEL_DESIGN, GLOBAL_HASHMAP)
    int threshold = global_heavy_hitter_tracker.stream_size.load(std::memory_order_relaxed) * delegation_sketch_context.app_configs.THETA;

    // collect the global heavy hitters
    if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy" &&
                  (DelegationBuildConfig::evaluate_accuracy_when == "start" || DelegationBuildConfig::evaluate_accuracy_when == "end")) {
        //  scan through the global heavy hitter tracker
        auto global_heavy_hitters = global_heavy_hitter_tracker.global_heavy_hitters.lock_table();

        for (const auto &it : global_heavy_hitters) {
            int key = it.first;
            int value = it.second;
            if (value >= threshold) { result[key] = value; }
        }

    } else if constexpr (DelegationBuildConfig::evaluate_mode == "throughput" ||
                         (DelegationBuildConfig::evaluate_mode == "accuracy" &&
                          (DelegationBuildConfig::evaluate_accuracy_when == "ivl" || DelegationBuildConfig::evaluate_accuracy_when == "end")) ||
                         DelegationBuildConfig::evaluate_mode == "latency") {
        //  scan through the global heavy hitter tracker
        // auto start_time = std::chrono::high_resolution_clock::now();

        // auto global_heavy_hitters = global_heavy_hitter_tracker.global_heavy_hitters.lock_table_nonblocking();

        // int key, value;
        // int count = 0;
        // for (const auto &it : global_heavy_hitters) {
        //     count++;
        //     key = it.first;
        //     value = it.second;
        //     // double collecting to check if key and value collected atomically
        //     while (it.first != key || it.second != value) {
        //         key = it.first;
        //         value = it.second;
        //     }
        //     if (value >= threshold) { result[key] = value; }
        // }

        auto global_heavy_hitters = global_heavy_hitter_tracker.global_heavy_hitters.lock_table_nonblocking();
        const auto &buckets = global_heavy_hitters.buckets();

        // Get raw access to the buckets array
        const auto *bucket_array = buckets.buckets_;
        const size_t num_buckets = buckets.size();
        const size_t slots_per_bucket = 4;   // SLOT_PER_BUCKET is hardcoded to 4 in the template

        // not initialized yet (need to check the concurrent cuckoo hash map later)
        if (num_buckets <= 1) { return; }

        // Direct memory access to values
        for (size_t i = 0; i < num_buckets; i++) {
            const auto &bucket = bucket_array[i];
            // Access the raw storage array values_ directly
            const auto &values = bucket.values_;

            // Iterate through values in the bucket
            for (size_t slot = 0; slot < slots_per_bucket; slot++) {
                // Get value directly from storage
                const auto &storage_kvpair = *static_cast<const std::pair<int, int> *>(static_cast<const void *>(&values[slot]));
                const int value = storage_kvpair.second;

                if (value >= threshold) { result[storage_kvpair.first] = value; }
            }
        }

        // auto end_time = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // std::cout << "Execution time: " << duration.count() << " microseconds for " << 1 << " items" << std::endl;
        // std::cout << "num_buckets: " << num_buckets << std::endl;
    }
#elif EQUAL(PARALLEL_DESIGN, QPOPSS)

    // collect the stream size
    int stream_size = 0;
    for (int i = 0; i < delegation_sketch_context.app_configs.NUM_THREADS; i++) {
        stream_size += thread_local_delegation_sketches[i]->thread_overall_stat_collector.count_received_from_stream_items;
    }

    // find heavy hitters
    int threshold = stream_size * delegation_sketch_context.app_configs.THETA;
    for (int i = 0; i < delegation_sketch_context.app_configs.NUM_THREADS; i++) {
        std::lock_guard<std::mutex> lock(thread_local_delegation_sketches[i]->QPOPSS_mutex);
        for (auto const &el : thread_local_delegation_sketches[i]->frequency_estimator.get_heavy_hitters()) {
            int key = el.first;
            int count = el.second;
            if (count >= threshold) { result[key] = count; }
        }
    }
#endif
}

template <typename FrequencyEstimator> template <typename T> float DelegationHeavyHitter<FrequencyEstimator>::ARE(const std::map<T, int> &exact_counter) {
    float relative_error = 0;
    for (const auto &entry : exact_counter) { relative_error += float(abs(entry.second - (int) this->direct_query(entry.first))) / entry.second; }
    return relative_error / exact_counter.size();
}

template <typename FrequencyEstimator> template <typename T> float DelegationHeavyHitter<FrequencyEstimator>::AAE(const std::map<T, int> &exact_counter) {
    float absolute_error = 0;
    for (const auto &entry : exact_counter) { absolute_error += float(abs(entry.second - (int) this->direct_query(entry.first))); }
    return absolute_error / exact_counter.size();
}

template <typename FrequencyEstimator>
template <typename T>
void DelegationHeavyHitter<FrequencyEstimator>::print_compare(const std::map<T, int> &exact_counter, string output_file_path) {
    ofstream output_file;
    if (!output_file_path.empty()) {
        output_file.open(output_file_path);
        if (!output_file.is_open()) {
            cerr << "Failed to open output file: " << output_file_path << endl;
            // Continue execution, but only output to cout
        }
    }

    // Function to print to both cout and file if specified
    auto print = [&output_file](const auto &x) {
        cout << x;
        if (output_file.is_open()) output_file << x;
    };

    for (const auto &entry : exact_counter) {
        // print exact counter and aprox counter to file
        print("Key: " + to_string(entry.first) + " Exact: " + to_string(entry.second) + " Approx: " + to_string(this->direct_query(entry.first)) + "\n");
    }
}
// ThreadLocalDelegationHeavyHitter implementation
template <typename FrequencyEstimator>
ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::ThreadLocalDelegationHeavyHitter(DelegationSketchContext &delegation_sketch_context, int current_thread_id,
                                                                                       FrequencyEstimator &frequency_estimator,
                                                                                       DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch)
    : delegation_sketch_context(delegation_sketch_context), local_heavy_hitter_tracker(delegation_sketch_context), current_thread_id(current_thread_id),
      frequency_estimator(frequency_estimator) {

    int num_threads = delegation_sketch_context.app_configs.NUM_THREADS;
    int filter_size = delegation_sketch_context.delegation_configs.FILTER_SIZE;
    this->delegation_sketch = delegation_sketch;
    this->delegation_filters = std::vector<DelegationFilter *>();
    this->pending_queries = std::vector<PendingQuery *>();
    this->thread_pairwise_stat_collectors = std::vector<ThreadPairWiseStatCollector>(num_threads);
    this->thread_overall_stat_collector = ThreadOverallStatCollector();
    this->seeds = seed_rand();

    for (int i = 0; i < num_threads; ++i) {
        this->double_buffer_delegation_filters[0].push_back(std::move(new DelegationFilter(filter_size)));
        this->double_buffer_delegation_filters[1].push_back(std::move(new DelegationFilter(filter_size)));
        current_buffer_ids.push_back(0);
        this->delegation_filters.push_back((DelegationFilter *) this->double_buffer_delegation_filters[0][i]);
        this->pending_queries.push_back(std::move(new PendingQuery()));
    }
}

template <typename FrequencyEstimator> void ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::process_pending_inserts() {
    if (full_delegate_filters.is_empty()) { return; }

#if EQUAL(PARALLEL_DESIGN, GLOBAL_HASHMAP)
    while (!full_delegate_filters.is_empty()) {
        DelegationFilter *filter;
        full_delegate_filters.pop(filter);

        int filter_size = filter->size.load(std::memory_order_relaxed);
        int total_differences = 0;
        for (int j = 0; j < filter_size; ++j) {
            total_differences += filter->counts[j];
            int count = this->frequency_estimator.update_and_estimate(filter->keys[j], filter->counts[j]);
            this->local_heavy_hitter_tracker.add_if_is_local_heavy_hitter(filter->keys[j], filter->counts[j], count);
            filter->counts[j] = 0;
            filter->keys[j] = 0;
        }

        filter->size = 0;
        filter->lock.store(false, std::memory_order_relaxed);
        this->local_heavy_hitter_tracker.update_global_heavy_hitters(this->delegation_sketch->global_heavy_hitter_tracker, total_differences);
    }
#elif EQUAL(PARALLEL_DESIGN, QPOPSS)

    if (!QPOPSS_mutex.try_lock()) { return; }

    while (!full_delegate_filters.is_empty()) {
        DelegationFilter *filter;
        full_delegate_filters.pop(filter);

        int total_differences = 0;
        int filter_size = filter->size.load(std::memory_order_relaxed);
        for (int j = 0; j < filter_size; ++j) {
            total_differences += filter->counts[j];
            this->frequency_estimator.update(filter->keys[j], filter->counts[j]);
            filter->counts[j] = 0;
            filter->keys[j] = 0;
        }

        filter->size = 0;
        filter->lock.store(false, std::memory_order_relaxed);

        int QPOPSS_stream_size = 0;
        // update stream size
        if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy" || DelegationBuildConfig::evaluate_mode == "latency") {
            QPOPSS_stream_size = this->delegation_sketch->QPOPSS_stream_size.fetch_add(total_differences);
            if (this->delegation_sketch->QPOPSS_stream_size.load() >= DelegationBuildConfig::evaluate_accuracy_stream_size) {
                if constexpr (DelegationBuildConfig::evaluate_accuracy_when == "start") { delegation_sketch_context.START_BENCHMARK.store(false, std::memory_order_relaxed); }
                delegation_sketch_context.START_ACCURACY_EVALUATION.store(true, std::memory_order_relaxed);
            }
        } else if constexpr (DelegationBuildConfig::evaluate_mode == "throughput") {
            QPOPSS_stream_size = this->delegation_sketch->QPOPSS_stream_size.fetch_add(total_differences);
        }

        this->frequency_estimator.update_threshold(QPOPSS_stream_size * delegation_sketch_context.app_configs.THETA);
    }

    QPOPSS_mutex.unlock();
#endif
}

template <typename FrequencyEstimator> void ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::flush_pending_inserts() {
    for (int i = 0; i < this->delegation_sketch_context.app_configs.NUM_THREADS; ++i) {
        auto filter = this->delegation_sketch->thread_local_delegation_sketches[i]->delegation_filters[current_thread_id];
        for (int j = 0; j < filter->size; ++j) {
            this->frequency_estimator.update(filter->keys[j], filter->counts[j]);
            filter->counts[j] = 0;
            filter->keys[j] = 0;
        }
        filter->size = 0;
    }
}

template <typename FrequencyEstimator> void ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::insert(const int &key) {
    int owner_thread_id = find_owner(key);

    this->thread_pairwise_stat_collectors[owner_thread_id].update_delegate_to_j_items();

    // if (owner_thread_id == current_thread_id) {
    //     this->insert_directly(key);
    //     return;
    // }

    auto filter = delegation_filters[owner_thread_id];
    bool flag = false;
    if (filter->lock.load(std::memory_order_relaxed) == true) {
        current_buffer_ids[owner_thread_id] = 1 - current_buffer_ids[owner_thread_id];
        delegation_filters[owner_thread_id] = double_buffer_delegation_filters[current_buffer_ids[owner_thread_id]][owner_thread_id];
        filter = delegation_filters[owner_thread_id];
        flag = true;

        this->thread_pairwise_stat_collectors[owner_thread_id].update_use_double_buffering();
    }

    while (filter->lock.load(std::memory_order_relaxed) == true && delegation_sketch_context.START_BENCHMARK) {
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
    if (filter->size.load(std::memory_order_relaxed) == this->delegation_sketch_context.delegation_configs.FILTER_SIZE) {
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

template <typename FrequencyEstimator> void ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::insert_directly(const int &key) { frequency_estimator.update(key); }

template <typename FrequencyEstimator> void ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::process_pending_queries() {
    return;
    for (int i = 0; i < this->delegation_sketch_context.app_configs.NUM_THREADS; ++i) {
        auto query = this->pending_queries[i];
        if (query->flag) {
            int count = 0;
            for (int j = 0; j < this->delegation_sketch_context.app_configs.NUM_THREADS; ++j) {
                auto &filter = this->delegation_sketch->thread_local_delegation_sketches[j]->delegation_filters[current_thread_id];
                count += filter->lookup_value_simd(query->key);
            }
            count += frequency_estimator.estimate(query->key);
            query->count = count;
            query->flag = false;

            for (int j = i + 1; j < this->delegation_sketch_context.app_configs.NUM_THREADS; ++j) {
                auto next_query = this->pending_queries[j];
                if (next_query->flag && next_query->key == query->key) {
                    next_query->count = count;
                    next_query->flag = false;
                }
            }
        }
    }
}

template <typename FrequencyEstimator> int ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::query(const int &key) {
    int owner_thread_id = find_owner(key);
    if (owner_thread_id == current_thread_id) { return this->query_directly(key); }
    auto query = this->delegation_sketch->thread_local_delegation_sketches[owner_thread_id]->pending_queries[current_thread_id];
    query->add_query(key);
    while (query->flag && delegation_sketch_context.START_BENCHMARK.load(std::memory_order_relaxed)) {
        process_pending_inserts();
        if (!delegation_sketch_context.delegation_configs.QUERY_RATE == 0) { process_pending_queries(); }
    }
    return query->count;
}

template <typename FrequencyEstimator> void ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::query_all_heavy_hitters(map<int, int> &results) {
    this->delegation_sketch->query_all_heavy_hitters(results);
}
template <typename FrequencyEstimator> int ThreadLocalDelegationHeavyHitter<FrequencyEstimator>::query_directly(const int &key) {
    int count = 0;
    for (int i = 0; i < this->delegation_sketch_context.app_configs.NUM_THREADS; ++i) {
        auto &filter = this->delegation_sketch->thread_local_delegation_sketches[i]->delegation_filters[current_thread_id];
        count += filter->lookup_value_simd(key);
    }
    count += frequency_estimator.estimate(key);

    for (int j = 0; j < this->delegation_sketch_context.app_configs.NUM_THREADS; ++j) {
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
void start_thread_heavy_hitter(DelegationSketchContext &delegation_sketch_context, ThreadLocalDelegationHeavyHitter<FrequencyEstimator> *thread_local_delegation_sketch, int start,
                               int end, std::barrier<> &sync_point) {
    setaffinity_oncpu(thread_local_delegation_sketch->current_thread_id + 2);
    sync_point.arrive_and_wait();
    while (delegation_sketch_context.START_BENCHMARK.load(std::memory_order_relaxed)) {
        for (int i = start; i < end; i++) {
            if (delegation_sketch_context.delegation_configs.QUERY_RATE &&
                should_perform_query(thread_local_delegation_sketch->seeds, delegation_sketch_context.delegation_configs.QUERY_RATE)) {
                unsigned int key = delegation_sketch_context.r1->tuples->at(i);
                int freq = thread_local_delegation_sketch->query(key);
            }
            if (delegation_sketch_context.delegation_configs.HEAVY_QUERY_RATE &&
                should_perform_query(thread_local_delegation_sketch->seeds, delegation_sketch_context.delegation_configs.HEAVY_QUERY_RATE)) {
                unsigned int key = delegation_sketch_context.r1->tuples->at(i);
                map<int, int> results;
                thread_local_delegation_sketch->query_all_heavy_hitters(results);
                // std::cout << "result size: " << results.size() << std::endl;
            } else {
                // std::cout << "no heavy query" << std::endl;
            }
            unsigned int key = delegation_sketch_context.r1->tuples->at(i);
            thread_local_delegation_sketch->insert(key);
            thread_local_delegation_sketch->thread_overall_stat_collector.update_received_from_stream_items();
            thread_local_delegation_sketch->process_pending_inserts();
            if (!delegation_sketch_context.START_BENCHMARK.load(std::memory_order_relaxed)) { break; }
        }
    }
}

template <typename FrequencyEstimator>
void start_accuracy_evaluator(DelegationSketchContext &delegation_sketch_context, DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch,
                              map<int, int> &accuracy_evaluator_heavy_hitter_counter, std::barrier<> &sync_point) {

    setaffinity_oncpu(1);
    sync_point.arrive_and_wait();

    while (delegation_sketch_context.START_ACCURACY_EVALUATION.load(std::memory_order_relaxed) == false);

    if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy") {

        if constexpr (DelegationBuildConfig::evaluate_accuracy_when == "start") { delegation_sketch_context.START_BENCHMARK.store(false, std::memory_order_relaxed); }

        delegation_sketch->query_all_heavy_hitters(accuracy_evaluator_heavy_hitter_counter);

        // stop the benchmark
        delegation_sketch_context.START_BENCHMARK.store(false, std::memory_order_relaxed);

        // collect the global heavy hitters at the end
        if constexpr (DelegationBuildConfig::evaluate_accuracy_when == "end") { delegation_sketch->query_all_heavy_hitters(accuracy_evaluator_heavy_hitter_counter); }

        delegation_sketch->accuracy_evaluator_heavy_hitter_counter = accuracy_evaluator_heavy_hitter_counter;
    } else if constexpr (DelegationBuildConfig::evaluate_mode == "latency") {

        // query all heavy hitters multiple times and measure the latency
        int NUM_QUERIES = 1000;
        while (delegation_sketch_context.START_BENCHMARK.load(std::memory_order_relaxed) && NUM_QUERIES-- > 0) {
            auto start_time = std::chrono::high_resolution_clock::now();
            map<int, int> results;
            delegation_sketch->query_all_heavy_hitters(results);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
            delegation_sketch->latency_evaluator_cache.push_back(duration.count());
        }

        // stop the benchmark
        delegation_sketch_context.START_BENCHMARK.store(false, std::memory_order_relaxed);
    }
};

template <typename FrequencyEstimator>
DelegationHeavyHitter<FrequencyEstimator> *start_threads(DelegationSketchContext &delegation_sketch_context, vector<FrequencyEstimator> &frequency_estimators) {
    vector<thread> threads;
    map<int, int> accuracy_evaluator_heavy_hitter_counter;

    int num_threads = delegation_sketch_context.app_configs.NUM_THREADS;
    int tuples_no = delegation_sketch_context.app_configs.tuples_no;
    int DURATION = delegation_sketch_context.app_configs.DURATION;
    int THETA = delegation_sketch_context.app_configs.THETA;

    // init DelegationSketch
    DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch = new DelegationHeavyHitter<FrequencyEstimator>(std::ref(delegation_sketch_context), frequency_estimators);

    // init sync_point and threads based on evaluate_mode
    const size_t barrier_count = (DelegationBuildConfig::evaluate_mode == "accuracy" || DelegationBuildConfig::evaluate_mode == "latency") ? num_threads + 2 : num_threads + 1;
    std::barrier sync_point(barrier_count);

    // if mode==accuracy, start the accuracy evaluator thread
    if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy" || DelegationBuildConfig::evaluate_mode == "latency") {
        threads.push_back(std::move(std::thread(start_accuracy_evaluator<FrequencyEstimator>, std::ref(delegation_sketch_context), delegation_sketch,
                                                std::ref(accuracy_evaluator_heavy_hitter_counter), std::ref(sync_point))));
    }

    // start threads
    for (int i = 0; i < num_threads; i++) {
        int start = i * (tuples_no / num_threads);
        int end = i == num_threads - 1 ? tuples_no : (i + 1) * (tuples_no / num_threads);
        cout << "thread: " << i << " start: " << start << " end: " << end << " end-start:" << end - start << endl;
        threads.push_back(std::move(std::thread(start_thread_heavy_hitter<FrequencyEstimator>, std::ref(delegation_sketch_context),
                                                delegation_sketch->thread_local_delegation_sketches[i], start, end, std::ref(sync_point))));
    }

    // start the benchmark
    delegation_sketch_context.START_BENCHMARK.store(true, std::memory_order_relaxed);
    sync_point.arrive_and_wait();
    start_time();

    if constexpr (DelegationBuildConfig::evaluate_mode == "latency") {
        // for latency
        if (DURATION > 0) {
            int check_interval = 100;   // Check every 100ms
            int total_ms = DURATION * 1000;

            while (total_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(check_interval));
                total_ms -= check_interval;
                if (!delegation_sketch_context.START_BENCHMARK.load(std::memory_order_relaxed)) { break; }
            }

            delegation_sketch_context.START_BENCHMARK.store(false, std::memory_order_relaxed);
            stop_time();
        }
    } else {

        // sleep for DURATION seconds and then stop the benchmark
        if (DURATION > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(DURATION));
            delegation_sketch_context.START_BENCHMARK.store(false, std::memory_order_relaxed);
            stop_time();
        }
    }

    // join threads
    for (int i = 0; i < threads.size(); i++) { threads[i].join(); }

    // process all pending inserts before joining
    for (int i = 0; i < num_threads; i++) {
        // process all pending inserts before print stats
        delegation_sketch->thread_local_delegation_sketches[i]->flush_pending_inserts();
    }

    return delegation_sketch;
}
template <typename FrequencyEstimator>
void print_stats_for_delegation_sketch(DelegationSketchContext &delegation_sketch_context, DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch, string output_file_path) {
    int num_threads = delegation_sketch_context.app_configs.NUM_THREADS;

    // Create an output file stream if output_file_path is specified
    ofstream output_file;
    if (!output_file_path.empty()) {
        output_file.open(output_file_path);
        if (!output_file.is_open()) {
            cerr << "Failed to open output file: " << output_file_path << endl;
            // Continue execution, but only output to cout
        }
    }

    // Function to print to both cout and file if specified
    auto print = [&output_file](const auto &x) {
        cout << x;
        if (output_file.is_open()) output_file << x;
    };

    // Rest of the function remains largely the same, replace cout with print

    long long total_insert_processed = 0, total_query_processed = 0, total_insert_processed_from_sketch = 0;
    long long total_need_to_wait = 0, total_no_need_to_wait = 0, total_count_looping_when_waiting = 0;

    vector<vector<ThreadPairWiseStatCollector>> all_thread_pairwise_stat_collectors;
    vector<ThreadOverallStatCollector> all_thread_overall_stat_collectors;

    for (int i = 0; i < num_threads; i++) {
        total_insert_processed += delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector.count_received_from_stream_items;
        total_insert_processed_from_sketch += delegation_sketch->thread_local_delegation_sketches[i]->frequency_estimator.total;
        total_query_processed += delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector.count_query_processed;
        print("thread " + to_string(i) +
              " proccessed:" + to_string(delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector.count_received_from_stream_items) + "\n");
        print("thread sketch " + to_string(i) + " proccessed:" + to_string(delegation_sketch->thread_local_delegation_sketches[i]->frequency_estimator.total) + "\n");
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
        print("----\n");
        all_thread_pairwise_stat_collectors.push_back(delegation_sketch->thread_local_delegation_sketches[i]->thread_pairwise_stat_collectors);
        all_thread_overall_stat_collectors.push_back(delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector);
    }

    std::string metrics_file_path = output_file_path.substr(0, output_file_path.find_last_of('.')) + "_metrics.json";
    print_all_threads_metrics_to_json(all_thread_pairwise_stat_collectors, all_thread_overall_stat_collectors, metrics_file_path);

    print("num_threads: " + to_string(delegation_sketch_context.app_configs.NUM_THREADS) + " total insert processed: " + to_string(float(total_insert_processed) / 1000000) +
          "Mops time process: " + to_string(get_time_ms() / 1000) + "\n");
    print("num_threads: " + to_string(delegation_sketch_context.app_configs.NUM_THREADS) + " total insert processed from sketch: " +
          to_string(float(total_insert_processed_from_sketch) / 1000000) + "Mops time process: " + to_string(get_time_ms() / 1000) + "\n");
    print("num_threads: " + to_string(delegation_sketch_context.app_configs.NUM_THREADS) + " total query processed: " + to_string(float(total_query_processed) / 1000000) +
          "Mops time process: " + to_string(get_time_ms() / 1000) + "\n");
    print("Throughput: " + to_string(float(total_insert_processed) / 1000000) + "\n");

    // Close the file if it was opened
    if (output_file.is_open()) { output_file.close(); }
}

template <typename FrequencyEstimator>
pair<map<int, int>, long> calculate_exact_counter(DelegationSketchContext &delegation_sketch_context, DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch) {
    Relation *r1 = delegation_sketch_context.r1;
    int num_threads = delegation_sketch_context.app_configs.NUM_THREADS;
    int tuples_no = delegation_sketch_context.app_configs.tuples_no;

    map<int, int> exact_counter;
    long total_processed = 0;

    for (int i = 0; i < num_threads; i++) {
        int start = i * (tuples_no / num_threads);
        int end = i == num_threads - 1 ? tuples_no : (i + 1) * (tuples_no / num_threads);
        int num_processed = delegation_sketch->thread_local_delegation_sketches[i]->thread_overall_stat_collector.count_received_from_stream_items;
        total_processed += num_processed;

        int weight = num_processed / (end - start);
        for (int j = start; j < end; j++) {
            unsigned int key = r1->tuples->at(j);
            exact_counter[key] += weight;
        }

        int rest = num_processed - (end - start) * weight;
        for (int j = start; j < start + rest; j++) {
            unsigned int key = r1->tuples->at(j);
            exact_counter[key]++;
        }
    }

    return {exact_counter, total_processed};
}

template <typename FrequencyEstimator, typename KeyType>
void print_stats_for_heavy_hitters(DelegationSketchContext &delegation_sketch_context, DelegationHeavyHitter<FrequencyEstimator> *delegation_sketch, string output_file_path) {
    int num_threads = delegation_sketch_context.app_configs.NUM_THREADS;
    int tuples_no = delegation_sketch_context.app_configs.tuples_no;
    float theta = delegation_sketch_context.app_configs.THETA;
    map<int, int> accuracy_evaluator_heavy_hitter_counter = delegation_sketch->accuracy_evaluator_heavy_hitter_counter;
    map<int, int> exact_counter;
    long total_processed = 0;
    tie(exact_counter, total_processed) = calculate_exact_counter<FrequencyEstimator>(std::ref(delegation_sketch_context), delegation_sketch);

    int threshold = total_processed * theta;

    map<int, int> heavy_hitter_counter;
    map<int, int> heavy_hitter_counter_2;
    int count_correct = 0;

    // Calculate heavy hitters and error
    for (auto el : exact_counter) {
        if (el.second > threshold) { heavy_hitter_counter[el.first] = el.second; }
    }

#if EQUAL(PARALLEL_DESIGN, GLOBAL_HASHMAP)
    if constexpr (DelegationBuildConfig::evaluate_mode == "throughput" || DelegationBuildConfig::evaluate_mode == "latency") {
        auto global_heavy_hitters = delegation_sketch->global_heavy_hitter_tracker.global_heavy_hitters.lock_table();
        for (const auto &top : global_heavy_hitters) {
            if (top.second >= threshold) {
                heavy_hitter_counter_2[top.first] = exact_counter[top.first];
                if (heavy_hitter_counter.count(top.first)) { count_correct += 1; }
            }
        }
    } else if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy") {
        for (const auto &top : accuracy_evaluator_heavy_hitter_counter) {
            heavy_hitter_counter_2[top.first] = exact_counter[top.first];
            if (heavy_hitter_counter.count(top.first)) { count_correct += 1; }
        }
    }
#elif EQUAL(PARALLEL_DESIGN, QPOPSS)
    if constexpr (DelegationBuildConfig::evaluate_mode == "throughput" || DelegationBuildConfig::evaluate_mode == "latency") {
        for (int i = 0; i < num_threads; i++) {
            for (auto const &el : delegation_sketch->thread_local_delegation_sketches[i]->frequency_estimator.get_heavy_hitters()) {
                int key = el.first;
                int count = el.second;
                if (count >= threshold) {
                    heavy_hitter_counter_2[key] = exact_counter[key];
                    if (heavy_hitter_counter.count(key)) { count_correct += 1; }
                }
            }
        }
    } else if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy") {
        for (const auto &top : accuracy_evaluator_heavy_hitter_counter) {
            heavy_hitter_counter_2[top.first] = exact_counter[top.first];
            if (heavy_hitter_counter.count(top.first)) { count_correct += 1; }
        }
    }
#endif

    // Create an output file stream if output_file_path is specified
    ofstream output_file;
    if (!output_file_path.empty()) {
        output_file.open(output_file_path);
        if (!output_file.is_open()) {
            cerr << "Failed to open output file: " << output_file_path << endl;
            // Continue execution, but only output to cout
        }
    }

    // Print results
    auto print_section = [&output_file](const std::string &title) {
        std::cout << "# " << title << std::endl;
        if (output_file.is_open()) output_file << "# " << title << std::endl;
    };

    auto print_metric = [&output_file](const std::string &name, const auto &value) {
        std::cout << name << ": " << value << std::endl;
        if (output_file.is_open()) output_file << name << ": " << value << std::endl;
    };
    size_t exact_count = exact_counter.size();
    size_t hh_count = heavy_hitter_counter.size();
    size_t hh_candidate_count = heavy_hitter_counter_2.size();
    // double are_hh = delegation_sketch->ARE(heavy_hitter_counter);
    // double aae_hh = delegation_sketch->AAE(heavy_hitter_counter);
    // double are_hh_candidates = delegation_sketch->ARE(heavy_hitter_counter_2);
    // double aae_hh_candidates = delegation_sketch->AAE(heavy_hitter_counter_2);

    double are_hh;
    double aae_hh;
    double are_hh_candidates;
    double aae_hh_candidates;

    // print exact counter and aprox counter to file
    size_t pos = output_file_path.find("_heavyhitter");
    string approx_counter_output_file_path = output_file_path;
    if (pos != string::npos) { approx_counter_output_file_path.replace(pos, 12, "_heavyhitter_appox"); }

    if constexpr (DelegationBuildConfig::evaluate_mode == "throughput" || DelegationBuildConfig::evaluate_mode == "latency" ||
                  (DelegationBuildConfig::evaluate_mode == "accuracy" && DelegationBuildConfig::evaluate_accuracy_error_sources == "algo")) {
        are_hh = delegation_sketch->ARE(heavy_hitter_counter);
        aae_hh = delegation_sketch->AAE(heavy_hitter_counter);
        delegation_sketch->print_compare(heavy_hitter_counter, approx_counter_output_file_path);
        are_hh_candidates = delegation_sketch->ARE(heavy_hitter_counter_2);
        aae_hh_candidates = delegation_sketch->AAE(heavy_hitter_counter_2);
        // no need to print this to calculate the error
        delegation_sketch->print_compare(heavy_hitter_counter_2, approx_counter_output_file_path);

    } else if constexpr (DelegationBuildConfig::evaluate_mode == "accuracy") {
        are_hh = ARE(heavy_hitter_counter, accuracy_evaluator_heavy_hitter_counter);
        aae_hh = AAE(heavy_hitter_counter, accuracy_evaluator_heavy_hitter_counter);
        print_compare(heavy_hitter_counter, accuracy_evaluator_heavy_hitter_counter, approx_counter_output_file_path);
        are_hh_candidates = ARE(heavy_hitter_counter_2, accuracy_evaluator_heavy_hitter_counter);
        aae_hh_candidates = AAE(heavy_hitter_counter_2, accuracy_evaluator_heavy_hitter_counter);
        // no need to print this to calculate the error
        print_compare(heavy_hitter_counter_2, accuracy_evaluator_heavy_hitter_counter, approx_counter_output_file_path);
    }
    double precision = static_cast<double>(count_correct) / hh_candidate_count;
    double recall = static_cast<double>(count_correct) / hh_count;

    print_section("sketch + heap heavy hitters");
    print_metric("Execution Time", delegation_sketch_context.app_configs.DURATION);
    print_metric("Count distinct", exact_count);
    print_metric("Total heavy hitters", hh_count);
    print_metric("Total heavy_hitter candidates", hh_candidate_count);

    print_section("sample set: true heavy_hitter_counter");
    print_metric("ARE", are_hh);
    print_metric("AAE", aae_hh);
    print_metric("Precision", std::to_string(count_correct) + " over " + std::to_string(hh_candidate_count));
    print_metric("Recall", std::to_string(count_correct) + " over " + std::to_string(hh_count));

    std::cout << "---------------------" << std::endl;

    print_section("sample set: heavy_hitter candidates");
    print_metric("ARE", are_hh_candidates);
    print_metric("AAE", aae_hh_candidates);

    std::cout << "---------------------" << std::endl;

    // latency evaluation
    if constexpr (DelegationBuildConfig::evaluate_mode == "latency") {
        print_section("Latency Evaluation");
        auto &latencies = delegation_sketch->latency_evaluator_cache;
        if (!latencies.empty()) {
            // Calculate statistics
            double avg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            auto max_it = std::max_element(latencies.begin(), latencies.end());
            auto min_it = std::min_element(latencies.begin(), latencies.end());

            print_metric("Number of measurements", latencies.size());
            print_metric("Average latency (ns)", avg);
            print_metric("Maximum latency (ns)", *max_it);
            print_metric("Minimum latency (ns)", *min_it);

            // Print raw latency values to file
            if (output_file.is_open()) {
                output_file << "Raw latencies (ns): ";
                for (size_t i = 0; i < latencies.size(); ++i) {
                    output_file << latencies[i];
                    if (i < latencies.size() - 1) { output_file << ","; }
                }
                output_file << std::endl;
            }
        }
    }

    std::cout << "RESULT_SUMMARY:"
              << " FrequencyEstimator=" << ConfigPrinter<FrequencyEstimator>::demangle(typeid(FrequencyEstimator).name()) << " TotalHeavyHitters=" << hh_count
              << " TotalHeavyHitterCandidates=" << hh_candidate_count << " Precision=" << precision << " Recall=" << recall << " ARE=" << are_hh << " AAE=" << aae_hh
              << " ExecutionTime=" << delegation_sketch_context.app_configs.DURATION << std::endl;
}