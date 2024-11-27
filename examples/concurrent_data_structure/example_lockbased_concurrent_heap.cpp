#include "concurrent_data_structure/LockBasedConcurrentHeap.hpp"
#include "frequency_estimator/BoundedKeyValuePriorityQueue.hpp"
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <thread>
#include <vector>

using namespace std;
// int main() {

//     vector<int> vec1;
//     vector<int> vec2;
//     vector<int> vec3;
//     LockBasedConcurrentHeap<int> heap;
//     for (int i = 0; i < 100; i++) {
//         // insert random numbers
//         int number = i;
//         int score = rand() % 100;
//         vec1.push_back(number);
//         vec2.push_back(score);
//         vec3.push_back(heap.push(number, score));
//         cout << "insert " << number << " with priority " << score << " at " << vec3[i] << endl;
//     }
//     cout << heap << endl;
//     for (int i = 90; i < 100; i++) {
//         // remove random numbers
//         cout << "change priority of " << vec1[i] << " at " << vec3[i] << " from " << vec2[i] << " to " << vec2[i] + 100 << endl;
//         heap.find_and_increase_priority(vec1[i], vec2[i], vec3[i], vec2[i] + 100);
//     }
//     cout << heap << endl;
//     return 0;
// }

constexpr int WORKLOAD = 100000;
constexpr int num_threads = 30;
constexpr int max_value = 1000;
struct Info {
    int number;
    int score;
    int index;
};

// Function to generate a Zipfian-distributed integer
int zipfian_random(int max_value, double skew) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0, 1);

    double z = dis(gen);
    double sum_prob = 0;
    double c = 0;

    // Calculate normalization constant c
    for (int i = 1; i <= max_value; i++) { c += 1.0 / pow(i, skew); }

    // Sample from the Zipfian distribution
    for (int i = 1; i <= max_value; i++) {
        sum_prob += (1.0 / pow(i, skew)) / c;
        if (z < sum_prob) { return i; }
    }
    return max_value;
}

int main() {
    // generate random numbers to vector in advance
    std::vector<int> numbers;
    for (int i = 0; i < WORKLOAD; i++) { numbers.push_back(zipfian_random(max_value, 1.5)); }

    // test concurrent heap
    std::vector<std::thread> threads;
    // start time
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    LockBasedConcurrentHeap<int> heap;
    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread([&heap, &numbers, i]() {
            std::map<int, Info> exists;
            for (int j = 0; j < WORKLOAD; j++) {
                // random
                // int number = rand() % 10 + i * 10;
                // int number = zipfian_random(10, 1.5) + i * 10;
                int number = numbers[j] + i * max_value;

                if (exists.find(number) != exists.end()) {
                    // change priority
                    int score = rand() % 5;
                    int index = heap.find_and_increase_priority(number, exists[number].index, exists[number].score + score).value();
                    exists[number] = {number, exists[number].score + score, index};
                    // cout << "change priority of " << number << " at " << index << " from " << exists[number].score - score << " to " << exists[number].score << endl;
                } else {
                    // insert
                    int score = rand() % 100;
                    int index = heap.push(number, score);
                    exists[number] = {number, score, index};
                    // cout << "insert " << number << " with priority " << score << " at " << index << endl;
                }
            }
        }));
    }
    // join all threads
    for (auto &thread : threads) { thread.join(); }
    // end time
    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    // test normal heap BoundedKeyValuePriorityQueue
    BoundedKeyValuePriorityQueue<int> heap2;
    std::vector<std::thread> threads2;
    // push using multithread using lock
    std::mutex mtx;
    chrono::steady_clock::time_point begin2 = chrono::steady_clock::now();
    for (int i = 0; i < num_threads; i++) {
        threads2.push_back(std::thread([&heap2, &numbers, &mtx, i]() {
            std::map<int, Info> exists;
            for (int j = 0; j < WORKLOAD; j++) {
                // random
                int number = numbers[j] + i * max_value;

                std::lock_guard<std::mutex> lock(mtx);
                if (exists.find(number) != exists.end()) {
                    // change priority
                    int score = rand() % 5;
                    heap2.update(number, exists[number].score + score);
                    exists[number] = {number, exists[number].score + score, exists[number].index};
                    // cout << "change priority of " << number << " at " << index << " from " << exists[number].score - score << " to " << exists[number].score << endl;
                } else {
                    // insert
                    int score = rand() % 100;
                    heap2.push(number, score);
                    exists[number] = {number, score, exists[number].index};
                    // cout << "insert " << number << " with priority " << score << " at " << index << endl;
                }
            }
        }));
    }

    // join all threads2
    for (auto &thread : threads2) { thread.join(); }
    chrono::steady_clock::time_point end2 = chrono::steady_clock::now();

    cout << "LockBasedConcurrentHeap: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
    // cout << heap << endl;
    cout << "BoundedKeyValuePriorityQueue: " << chrono::duration_cast<chrono::milliseconds>(end2 - begin2).count() << "ms" << endl;
    // cout << heap2 << endl;
}