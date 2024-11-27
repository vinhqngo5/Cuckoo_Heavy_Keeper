#pragma once
#include <atomic>
#include <memory>
#include <optional>
#include <vector>

template <typename T> class TreiberStack {
  private:
    struct Node {
        T data;
        std::shared_ptr<Node> next;
        Node(const T &value) : data(value), next(nullptr) {}
    };

    std::atomic<std::shared_ptr<Node>> head;

  public:
    TreiberStack() : head(nullptr) {}

    void push(const T &value) {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        std::shared_ptr<Node> old_head;
        do {
            old_head = head.load(std::memory_order_relaxed);
            new_node->next = old_head;
        } while (!head.compare_exchange_weak(old_head, new_node, std::memory_order_release,
                                             std::memory_order_relaxed));
    }

    bool pop(T &result) {
        std::shared_ptr<Node> old_head;
        do {
            old_head = head.load(std::memory_order_relaxed);
            if (old_head == nullptr) {
                return false;   // Stack is empty
            }
        } while (!head.compare_exchange_weak(old_head, old_head->next, std::memory_order_acquire,
                                             std::memory_order_relaxed));
        result = old_head->data;
        return true;
    }

    bool is_empty() const { return head.load(std::memory_order_relaxed) == nullptr; }
};
