#include <algorithm>
#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <vector>

template <typename T> class LockBasedConcurrentHeap {
    static_assert(std::is_same<decltype(std::declval<std::ostream &>() << std::declval<T>()), std::ostream &>::value, "T must be streamable to std::ostream");

  public:
    LockBasedConcurrentHeap(const LockBasedConcurrentHeap &other) {
        std::unique_lock<std::shared_mutex> lock(other.mutex);
        heap = other.heap;
    }

    LockBasedConcurrentHeap() = default;

  private:
    struct Node {
        T item;
        int score;
        Node(T i, int s) : item(std::move(i)), score(s) {}
        friend std::ostream &operator<<(std::ostream &os, const Node &node) {
            os << "(" << node.item << ": " << node.score << ")";
            return os;
        }
    };

    std::vector<Node> heap;
    mutable std::shared_mutex mutex;

    int heapify_up(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[parent].score <= heap[index].score) break;
            std::swap(heap[parent], heap[index]);
            index = parent;
        }
        return index;
    }

    int heapify_down(int index) {
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int smallest = index;

            if (left < heap.size() && heap[left].score < heap[smallest].score) smallest = left;
            if (right < heap.size() && heap[right].score < heap[smallest].score) smallest = right;

            if (smallest == index) break;

            std::swap(heap[index], heap[smallest]);
            index = smallest;
        }
        return index;
    }

    std::optional<int> find_item(const T &item, int old_index = 0) const {
        if (item == heap[old_index].item) {
            // std::cout << "Found item at old index " << old_index << std::endl;
            return old_index;
        }
        for (int i = 0; i < heap.size(); i++) {
            if (item == heap[i].item) {
                // std::cout << "Found item at new index " << i << std::endl;
                return i;
            }
        }
        // std::cout << "Item not found" << std::endl;
        return std::nullopt;
    }

  public:
    int push(const T &item, int score) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        heap.emplace_back(item, score);
        return heapify_up(heap.size() - 1);
    }

    std::optional<Node> pop() {
        std::unique_lock<std::shared_mutex> lock(mutex);
        if (heap.empty()) return std::nullopt;

        Node node = heap[0];
        heap[0] = heap.back();
        heap.pop_back();

        if (!heap.empty()) { heapify_down(0); }

        return node;
    }

    std::optional<int> find_and_increase_priority(const T &item, int old_index, int new_score) {
        if (old_index >= heap.size()) { throw std::out_of_range("Start index out of range"); }

        std::optional<int> found_index;
        {
            std::shared_lock<std::shared_mutex> shared_lock(mutex);
            found_index = find_item(item, old_index);
            if (!found_index.has_value()) {
                std::cout << "Item not found" << std::endl;
                return std::nullopt;
            }
        }

        int item_index = found_index.value();
        bool need_heapify = false;
        {
            std::shared_lock<std::shared_mutex> shared_lock(mutex);
            int left = 2 * item_index + 1;
            int right = 2 * item_index + 2;
            need_heapify = (left < heap.size() && new_score > heap[left].score) || (right < heap.size() && new_score > heap[right].score);

            if (!need_heapify) {
                heap[item_index].score = new_score;
                // std::cout << "no need to heapify" << std::endl;
                return item_index;
            }
        }

        // if need heapify
        // std::cout << "need to heapify" << std::endl;
        std::unique_lock<std::shared_mutex> exclusive_lock(mutex);

        // if there are interleaving pushions/deletions/change priority, we need to find the item again
        if (item != heap[item_index].item) { item_index = find_item(item, item_index).value(); }
        heap[item_index].score = new_score;

        return heapify_down(item_index);
    }

    int size() const {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return heap.size();
    }

    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return heap.empty();
    }

    friend std::ostream &operator<<(std::ostream &os, const LockBasedConcurrentHeap &heap) {
        // Create a copy of the heap
        LockBasedConcurrentHeap heap_copy = heap;

        os << "Heap: [";
        std::optional<Node> node;
        bool first = true;
        while ((node = heap_copy.pop()).has_value()) {
            if (!first) { os << ", "; }
            os << node.value();
            first = false;
        }
        os << "]";
        return os;
    }
};