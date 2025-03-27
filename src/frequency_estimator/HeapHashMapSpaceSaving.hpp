#pragma once

#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// HeapHashMapSpaceSaving using naive heapify
class HeapHashMapSpaceSaving : public FrequencyEstimatorBase {
  public:
    struct Item {
        std::string name;
        unsigned int count;
        Item(const std::string &n, unsigned int c) : name(n), count(c) {}
    };

  private:
    struct CompareItem {
        bool operator()(const Item *a, const Item *b) {
            return a->count > b->count;   // Min-heap
        }
    };

    int k;
    std::unordered_map<std::string, Item *> item_map;
    std::priority_queue<Item *, std::vector<Item *>, CompareItem> min_heap;

    std::string int_to_string(const int &item) { return std::to_string(item); }

    void _update(const std::string &item, int c) {
        auto it = item_map.find(item);
        if (it != item_map.end()) {
            it->second->count += c;
            // Reheapify
            std::make_heap(const_cast<Item **>(&min_heap.top()), const_cast<Item **>(&min_heap.top()) + min_heap.size(), CompareItem());
        } else if (item_map.size() < k) {
            Item *new_item = new Item(item, c);
            item_map[item] = new_item;
            min_heap.push(new_item);
        } else {
            Item *min_item = min_heap.top();
            min_heap.pop();
            item_map.erase(min_item->name);

            min_item->name = item;
            min_item->count += c;

            item_map[item] = min_item;
            min_heap.push(min_item);
        }
    }

  public:
    HeapHashMapSpaceSaving(int k) : k(k) {}

    HeapHashMapSpaceSaving(SpaceSavingConfig &config) : k(config.K) {}

    ~HeapHashMapSpaceSaving() {
        for (auto &pair : item_map) { delete pair.second; }
    }

    void print_status() override {
        std::cout << "Size: " << k << std::endl;
        std::cout << "Estimated size in bytes: " << k * (sizeof(Item) + sizeof(std::string) + sizeof(void *)) << std::endl;
    }

    void update(const int &item, int c = 1) override { _update(int_to_string(item), c); }

    void update(const std::string &item, int c = 1) override { _update(item, c); }

    unsigned int estimate(const int &item) override { return estimate(int_to_string(item)); }

    unsigned int estimate(const std::string &item) override {
        auto it = item_map.find(item);
        return (it != item_map.end()) ? it->second->count : 0;
    }

    unsigned int update_and_estimate(const int &item, int c = 1) override {
        update(item, c);
        return estimate(item);
    }

    unsigned int update_and_estimate(const std::string &item, int c = 1) override {
        update(item, c);
        return estimate(item);
    }

    class Iterator {
      private:
        std::unordered_map<std::string, Item *>::const_iterator it;

      public:
        Iterator(std::unordered_map<std::string, Item *>::const_iterator iterator) : it(iterator) {}

        Iterator &operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const Iterator &other) const { return it != other.it; }

        std::pair<std::string, unsigned int> operator*() const { return {it->first, it->second->count}; }
    };

    // Iterator access methods
    Iterator begin() const { return Iterator(item_map.begin()); }

    Iterator end() const { return Iterator(item_map.end()); }
};

// V2 is faster compared to V3 due to using pointer to Item instead of placing item directly in the
// vector
class HeapHashMapSpaceSavingV2 : public FrequencyEstimatorBase {
  private:
    struct Item {
        std::string name;
        unsigned int count;
        int heap_index;
        Item(const std::string &n, unsigned int c, int idx) : name(n), count(c), heap_index(idx) {}
    };

    int k;
    std::unordered_map<std::string, Item *> item_map;
    std::vector<Item *> heap;

    std::string int_to_string(const int &item) { return std::to_string(item); }

    void sift_up(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[index]->count < heap[parent]->count) {
                std::swap(heap[index], heap[parent]);
                heap[index]->heap_index = index;
                heap[parent]->heap_index = parent;
                index = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(int index) {
        while (true) {
            int min_index = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < heap.size() && heap[left]->count < heap[min_index]->count) { min_index = left; }
            if (right < heap.size() && heap[right]->count < heap[min_index]->count) { min_index = right; }

            if (min_index != index) {
                std::swap(heap[index], heap[min_index]);
                heap[index]->heap_index = index;
                heap[min_index]->heap_index = min_index;
                index = min_index;
            } else {
                break;
            }
        }
    }

    void _update(const std::string &item, int c) {
        total += c;
        auto it = item_map.find(item);
        if (it != item_map.end()) {
            Item *updated_item = it->second;
            updated_item->count += c;
            sift_down(updated_item->heap_index);
        } else if (heap.size() < k) {
            Item *new_item = new Item(item, c, heap.size());
            item_map[item] = new_item;
            heap.push_back(new_item);
            sift_up(heap.size() - 1);
        } else {
            Item *min_item = heap[0];
            item_map.erase(min_item->name);

            min_item->name = item;
            min_item->count += c;

            item_map[item] = min_item;
            sift_down(0);
        }
    }

  public:
    unsigned int total = 0;

    HeapHashMapSpaceSavingV2(int k) : k(k) { heap.reserve(k); }

    HeapHashMapSpaceSavingV2(SpaceSavingConfig &config) : k(config.K) { heap.reserve(k); }

    ~HeapHashMapSpaceSavingV2() {
        for (auto &pair : item_map) { delete pair.second; }
    }

    void print_status() override {
        std::cout << "Size: " << k << std::endl;
        std::cout << "Estimated size in bytes: " << k * (sizeof(Item) + sizeof(std::string) + 2 * sizeof(void *)) << std::endl;
    }

    void update(const int &item, int c = 1) override { _update(int_to_string(item), c); }

    void update(const std::string &item, int c = 1) override { _update(item, c); }

    unsigned int estimate(const int &item) override { return estimate(int_to_string(item)); }

    unsigned int estimate(const std::string &item) override {
        auto it = item_map.find(item);
        return (it != item_map.end()) ? it->second->count : 0;
    }

    unsigned int update_and_estimate(const int &item, int c = 1) override {
        update(item, c);
        return estimate(item);
    }

    unsigned int update_and_estimate(const std::string &item, int c = 1) override {
        update(item, c);
        return estimate(item);
    }

    class Iterator {
      private:
        std::vector<Item *>::const_iterator it;

      public:
        Iterator(std::vector<Item *>::const_iterator iterator) : it(iterator) {}

        Iterator &operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const Iterator &other) const { return it != other.it; }

        const Item *operator*() const { return *it; }
    };

    // Begin and end methods for the container
    Iterator begin() const { return Iterator(heap.begin()); }

    Iterator end() const { return Iterator(heap.end()); }
};

class HeapHashMapSpaceSavingV3 : public FrequencyEstimatorBase {
  private:
    struct Item {
        uint32_t count;
        uint32_t name_index;   // Index into names vector
    };

    int k;
    std::vector<Item> items;
    std::vector<std::string> names;
    std::unordered_map<std::string, int> item_map;   // Maps names to indices in items

    void sift_up(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (items[index].count < items[parent].count) {
                std::swap(items[index], items[parent]);
                item_map[names[items[index].name_index]] = index;
                item_map[names[items[parent].name_index]] = parent;
                index = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(int index) {
        while (true) {
            int min_index = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < items.size() && items[left].count < items[min_index].count) { min_index = left; }
            if (right < items.size() && items[right].count < items[min_index].count) { min_index = right; }

            if (min_index != index) {
                std::swap(items[index], items[min_index]);
                item_map[names[items[index].name_index]] = index;
                item_map[names[items[min_index].name_index]] = min_index;
                index = min_index;
            } else {
                break;
            }
        }
    }

    std::string int_to_string(const int &item) { return std::to_string(item); }

  public:
    HeapHashMapSpaceSavingV3(int k) : k(k) {
        items.reserve(k);
        names.reserve(k);
    }

    HeapHashMapSpaceSavingV3(SpaceSavingConfig &config) : k(config.K) {
        items.reserve(k);
        names.reserve(k);
    }

    void _update(const std::string &item, int c = 1) {
        auto it = item_map.find(item);
        if (it != item_map.end()) {
            int index = it->second;
            items[index].count += c;
            sift_down(index);
        } else if (items.size() < k) {
            names.push_back(item);
            items.push_back({static_cast<uint32_t>(c), static_cast<uint32_t>(names.size() - 1)});
            item_map[item] = items.size() - 1;
            sift_up(items.size() - 1);
        } else {
            Item &min_item = items[0];
            item_map.erase(names[min_item.name_index]);
            names[min_item.name_index] = item;
            min_item.count += c;
            item_map[item] = 0;
            sift_down(0);
        }
    }

  public:
    void update(const int &item, int c = 1) override { _update(int_to_string(item), c); }

    void update(const std::string &item, int c = 1) override { _update(item, c); }

    unsigned int estimate(const int &item) override { return estimate(int_to_string(item)); }

    unsigned int estimate(const std::string &item) override {
        auto it = item_map.find(item);
        return (it != item_map.end()) ? items[it->second].count : 0;
    }

    unsigned int update_and_estimate(const int &item, int c = 1) override {
        update(item, c);
        return estimate(item);
    }

    unsigned int update_and_estimate(const std::string &item, int c = 1) override {
        update(item, c);
        return estimate(item);
    }
    void print_status() {
        std::cout << "Size: " << k << std::endl;
        std::cout << "Estimated size in bytes: " << (k * (sizeof(Item) + sizeof(std::string)) + sizeof(std::unordered_map<std::string, int>)) << std::endl;
    }
    class Iterator {
      private:
        std::unordered_map<std::string, int>::const_iterator it;
        const HeapHashMapSpaceSavingV3 *parent;

      public:
        Iterator(std::unordered_map<std::string, int>::const_iterator it, const HeapHashMapSpaceSavingV3 *parent) : it(it), parent(parent) {}

        Iterator &operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const Iterator &other) const { return it != other.it; }

        std::pair<std::string, unsigned int> operator*() const { return {it->first, parent->items[it->second].count}; }
    };

    // Iterator methods
    Iterator begin() const { return Iterator(item_map.begin(), this); }

    Iterator end() const { return Iterator(item_map.end(), this); }
};