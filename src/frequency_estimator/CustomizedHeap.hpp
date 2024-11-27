#pragma once

#include <queue>

template <typename T> struct MaxFrequencyComparer {
    bool operator()(const T &lhs, const T &rhs) const {
        return lhs.second > rhs.second;   // Compare based on the 'int' value
    }
    bool equal(const T &lhs, const T &rhs) const {
        return lhs.first == rhs.first;   // Compare for equality based on the 'int' value
    }
};

// This class is a customized heap used for finding top K elements
template <typename T, typename container = std::vector<T>,
          typename compare = std::less<typename container::value_type>>
class CustomizedHeap {
  private:
    std::priority_queue<T, container, compare> pq;

  public:
    bool is_set_limit = false;
    int limit = -1;

  public:
    // Wrapper methods to forward the operations to the underlying queue
    void push(const T &item) {
        this->pq.push(item);
        // Pop the last element if the size exceeds the limit
        if (this->is_set_limit && this->pq.size() > this->limit) { this->pq.pop(); }
    }

    T top() const { return this->pq.top(); }

    void pop() { this->pq.pop(); }

    bool empty() const { return this->pq.empty(); }

    size_t size() const { return this->pq.size(); }

    // Extended methods
    // Add limit for queue, auto evict when size exceeds
    void set_limit(int limit) {
        this->limit = limit;
        this->is_set_limit = true;
    }

    int get_limit() const { return this->limit; }

    // update if existed else push
    void update(const T &item) {
        compare comparer;
        // check if need to update (if min in heap is already bigger than c -> skip)
        if (this->is_set_limit && this->pq.size() == this->limit) {
            T top_item = this->pq.top();
            bool result = comparer(item, top_item);
            // if freq of item is not bigger than freq of topItem -> return
            if (!result) { return; }
        }

        std::priority_queue<T, container, compare> temp_queue = pq;
        bool is_exist = false;

        // check if exist in pq
        while (!temp_queue.empty()) {
            T top_item = temp_queue.top();
            temp_queue.pop();
            if (comparer.equal(top_item, item)) {
                is_exist = true;
                break;
            }
        }

        // if exist
        if (is_exist) {
            std::priority_queue<T, container, compare> temp_queue2;
            // Search for the item to update
            while (!this->pq.empty()) {
                T top_item = this->pq.top();
                this->pq.pop();
                if (comparer.equal(top_item, item)) { continue; }
                temp_queue2.push(top_item);
            }
            this->pq = temp_queue2;
        }

        this->push(item);
    }
};