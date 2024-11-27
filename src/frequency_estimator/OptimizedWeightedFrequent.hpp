#pragma once

#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include <queue>
#include <unordered_map>
#include <vector>

#define LONG_PRIME 2147483647

class OptimizedWeightedFrequent : public FrequencyEstimatorBase {
  public:
    OptimizedWeightedFrequent() = default;
    OptimizedWeightedFrequent(float epsilon) : epsilon(epsilon), n(ceil(1.0 / epsilon)), M(0) {}
    OptimizedWeightedFrequent(int n) : n(n), epsilon(1.0 / n), M(0) {}
    OptimizedWeightedFrequent(const WeightedFrequentConfig &config) {
        if (config.CALCULATE_FROM == "EPSILON") {
            epsilon = config.EPSILON;
            n = ceil(1.0 / epsilon);
        } else if (config.CALCULATE_FROM == "N") {
            n = config.N;
            epsilon = 1.0 / n;
        }
    }

    void update(const int &item, int c = 1) override { _update(item, c); }
    void update(const string &item, int c = 1) override { _update(_get_hashitem(item), c); }

    unsigned int estimate(const int &item) override { return _estimate(item); }
    unsigned int estimate(const string &item) override { return _estimate(_get_hashitem(item)); }

    unsigned int update_and_estimate(const int &item, int c = 1) override {
        _update(item, c);
        return _estimate(item);
    }

    unsigned int update_and_estimate(const string &item, int c = 1) override {
        unsigned int hashitem = _get_hashitem(item);
        _update(hashitem, c);
        return _estimate(hashitem);
    }

    void print_status() override {
        // Implementation depends on what you want to print
        std::cout << "OptimizedWeightedFrequent" << " heap size:" << heap.size() << std::endl;
    }

  private:
    float epsilon;
    int n;
    long long M;
    std::unordered_map<int, int> item_to_index;
    std::vector<std::pair<int, int>> heap;   // (item, frequency)

    unsigned int _get_hashitem(const string &item) {
        unsigned long hash = 5381;
        for (char c : item) { hash = (((hash << 5) + hash) + c) % LONG_PRIME; }
        return hash;
    }

    void _update(const int &item, int c) {
        if (item_to_index.find(item) != item_to_index.end()) {
            int index = item_to_index[item];
            heap[index].second += c;
            _heapify_down(index);
        } else if (heap.size() < n) {
            item_to_index[item] = heap.size();
            heap.emplace_back(item, M + c);
            _heapify_up(heap.size() - 1);
        } else {
            int c_min = heap[0].second - M;
            if (c >= c_min) {
                M += c_min;
                item_to_index.erase(heap[0].first);
                heap[0] = {item, c - c_min};
                item_to_index[item] = 0;
                _heapify_down(0);

                // Remove additional items smaller than M
                while (!heap.empty() && heap[0].second <= M) {
                    item_to_index.erase(heap[0].first);
                    if (heap.size() > 1) {
                        heap[0] = heap.back();
                        item_to_index[heap[0].first] = 0;
                        heap.pop_back();
                        _heapify_down(0);
                    } else {
                        heap.pop_back();
                    }
                }
            } else {
                M += c;
            }
        }
    }

    unsigned int _estimate(const int &item) {
        if (item_to_index.find(item) != item_to_index.end()) {
            int index = item_to_index[item];
            return heap[index].second - M;
        }
        return 0;
    }

    void _heapify_up(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[parent].second > heap[index].second) {
                std::swap(heap[parent], heap[index]);
                item_to_index[heap[parent].first] = parent;
                item_to_index[heap[index].first] = index;
                index = parent;
            } else {
                break;
            }
        }
    }

    void _heapify_down(int index) {
        while (true) {
            int smallest = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < heap.size() && heap[left].second < heap[smallest].second) { smallest = left; }
            if (right < heap.size() && heap[right].second < heap[smallest].second) { smallest = right; }

            if (smallest != index) {
                std::swap(heap[index], heap[smallest]);
                item_to_index[heap[index].first] = index;
                item_to_index[heap[smallest].first] = smallest;
                index = smallest;
            } else {
                break;
            }
        }
    }
};
