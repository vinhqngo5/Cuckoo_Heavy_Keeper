#pragma once

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

template <typename KeyType> class BoundedKeyValuePriorityQueue {
  private:
    struct HeapElement {
        KeyType key;
        int weight;

        HeapElement(const KeyType &k, int w) : key(k), weight(w) {}

        friend std::ostream &operator<<(std::ostream &os, const HeapElement &element) {
            os << "(" << element.key << ": " << element.weight << ")";
            return os;
        }
    };

    std::vector<HeapElement> heap;
    std::unordered_map<KeyType, size_t> key_to_index;
    size_t max_size;
    bool is_bounded;

    void sift_up(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (heap[index].weight < heap[parent].weight) {
                std::swap(heap[index], heap[parent]);
                key_to_index[heap[index].key] = index;
                key_to_index[heap[parent].key] = parent;
                index = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(size_t index) {
        size_t size = heap.size();
        while (true) {
            size_t smallest = index;
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;

            if (left < size && heap[left].weight < heap[smallest].weight) smallest = left;
            if (right < size && heap[right].weight < heap[smallest].weight) smallest = right;

            if (smallest != index) {
                std::swap(heap[index], heap[smallest]);
                key_to_index[heap[index].key] = index;
                key_to_index[heap[smallest].key] = smallest;
                index = smallest;
            } else {
                break;
            }
        }
    }

  public:
    BoundedKeyValuePriorityQueue(size_t max_size = 0) : max_size(max_size), is_bounded(max_size > 0) {}

    void push(const KeyType &key, int weight) {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            update(key, weight);
            return;
        }

        if (is_bounded && heap.size() >= max_size) {
            if (weight > heap[0].weight) {
                key_to_index.erase(heap[0].key);
                heap[0] = HeapElement(key, weight);
                key_to_index[key] = 0;
                sift_down(0);
            }
        } else {
            heap.push_back(HeapElement(key, weight));
            key_to_index[key] = heap.size() - 1;
            sift_up(heap.size() - 1);
        }
    }

    void update(const KeyType &key, int weight) {
        auto it = key_to_index.find(key);
        if (it == key_to_index.end()) {
            push(key, weight);
            return;
        }

        size_t index = it->second;
        int old_weight = heap[index].weight;
        heap[index].weight = weight;

        if (weight < old_weight) {
            sift_up(index);
        } else {
            sift_down(index);
        }
    }

    void update_add(const KeyType &key, int weight_change) {
        auto it = key_to_index.find(key);
        if (it == key_to_index.end()) {
            push(key, weight_change);
            return;
        }

        size_t index = it->second;
        heap[index].weight += weight_change;

        if (weight_change < 0) {
            sift_up(index);
        } else {
            sift_down(index);
        }
    }

    void pop() {
        if (empty()) return;
        key_to_index.erase(heap[0].key);
        if (heap.size() > 1) {
            heap[0] = heap.back();
            key_to_index[heap[0].key] = 0;
            heap.pop_back();
            sift_down(0);
        } else {
            heap.pop_back();
        }
    }

    // pop all items with priority < weight and return the list of popped items
    std::vector<std::pair<KeyType, int>> pop_all_below(int weight) {
        std::vector<std::pair<KeyType, int>> popped_items;
        while (!empty() && heap[0].weight < weight) {
            popped_items.emplace_back(heap[0].key, heap[0].weight);
            key_to_index.erase(heap[0].key);
            if (heap.size() > 1) {
                heap[0] = heap.back();
                key_to_index[heap[0].key] = 0;
                heap.pop_back();
                sift_down(0);
            } else {
                heap.pop_back();
            }
        }
        return popped_items;
    }

    std::pair<KeyType, int> top() const {
        if (empty()) throw std::runtime_error("Queue is empty");
        return {heap[0].key, heap[0].weight};
    }

    bool empty() const { return heap.empty(); }

    bool is_full() const { return is_bounded && heap.size() == max_size; }

    size_t size() const { return heap.size(); }

    void set_bound(size_t new_max_size) {
        max_size = new_max_size;
        is_bounded = true;

        while (heap.size() > max_size) {
            key_to_index.erase(heap.back().key);
            heap.pop_back();
        }
        std::make_heap(heap.begin(), heap.end(), [](const HeapElement &a, const HeapElement &b) { return a.weight > b.weight; });
        for (size_t i = 0; i < heap.size(); ++i) { key_to_index[heap[i].key] = i; }
    }

    void remove_bound() {
        max_size = 0;
        is_bounded = false;
    }

    bool get_is_bounded() const { return is_bounded; }

    size_t get_bound() const { return max_size; }

    bool contains(const KeyType &key) const { return key_to_index.find(key) != key_to_index.end(); }

    friend std::ostream &operator<<(std::ostream &os, BoundedKeyValuePriorityQueue bkpq) {
        if (bkpq.empty()) {
            os << "Empty Queue";
            return os;
        }

        while (!bkpq.empty()) {
            os << bkpq.top().first << ": " << bkpq.top().second << " ";
            bkpq.pop();
        }

        return os;
    }

    // Iterator class
    class Iterator {
      private:
        typename std::vector<HeapElement>::const_iterator it;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<const KeyType, int>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type *;
        using reference = const value_type &;

        Iterator(typename std::vector<HeapElement>::const_iterator iter) : it(iter) {}

        value_type operator*() const { return {it->key, it->weight}; }
        pointer operator->() const {
            static value_type pair;
            pair = {it->key, it->weight};
            return &pair;
        }

        Iterator &operator++() {
            ++it;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator &other) const { return it == other.it; }
        bool operator!=(const Iterator &other) const { return it != other.it; }
    };

    // Iterator methods
    Iterator begin() const { return Iterator(heap.begin()); }
    Iterator end() const { return Iterator(heap.end()); }

    // Const iterator methods
    using const_iterator = Iterator;
    const_iterator cbegin() const { return const_iterator(heap.cbegin()); }
    const_iterator cend() const { return const_iterator(heap.cend()); }
};
