#include <cassert>   // Add this line to include the <cassert> header
#include <iostream>
#include <thread>
#include <type_traits>

constexpr int WORKLOAD = 10000;

template <typename T> struct DefaultValue {
    static T get() {
        return T();   // Default constructor for fundamental types, etc.
    }
};

// Specialization for pointer types
template <typename T> struct DefaultValue<T *> {
    static T *get() {
        return new T();   // Default value for pointer types
    }
};

template <template <typename> class T, typename U> void test_concurrent_queue(int num_threads) {
    T<U> queue = T<U>();

    int pop_count = 0;
    std::atomic<int> count_finished_threads(0);

    // create threads to push data
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread([&queue, &count_finished_threads, i]() {
            for (int j = 0; j < WORKLOAD; j++) { queue.push(DefaultValue<U>::get()); }
            count_finished_threads++;
        }));
    }

    // create threads to pop data
    std::vector<int> thread_local_pop_counts(num_threads,
                                             0);   // Vector to hold pop counts per thread

    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread(
            [&queue, &thread_local_pop_counts, &count_finished_threads, &num_threads, i]() {
                int thread_local_pop_count = 0;
                while (!queue.is_empty() || count_finished_threads.load() < num_threads) {
                    U value;
                    if (queue.pop(value)) { thread_local_pop_count++; }
                }
                thread_local_pop_counts[i] = thread_local_pop_count;
            }));
    }

    // join all threads
    for (auto &thread : threads) { thread.join(); }

    // calculate total pop count
    for (int i = 0; i < num_threads; i++) { pop_count += thread_local_pop_counts[i]; }

    // print the size of the queue
    std::cout << "Queue is empty: " << queue.is_empty() << std::endl;
    for (int i = 0; i < num_threads; i++) {
        std::cout << "Thread " << i << " popped " << thread_local_pop_counts[i] << " elements"
                  << std::endl;
    }
    std::cout << "Total number of elements popped: " << pop_count << std::endl;

    // assert that all elements have been popped
    assert(pop_count == num_threads * WORKLOAD);
}