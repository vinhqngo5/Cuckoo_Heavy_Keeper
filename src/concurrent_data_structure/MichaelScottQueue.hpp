#pragma once
#include <atomic>
#include <optional>
#include <vector>

template <typename T> class MichaelScottQueue {
  public:
    MichaelScottQueue() {
        Node *dummy = new Node(T());
        head.store(dummy);
        tail.store(dummy);
    }

    ~MichaelScottQueue() {
        while (Node *node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }

    void enqueue(T value) {
        Node *new_node = new Node(value);
        Node *old_tail = nullptr;

        while (true) {
            old_tail = tail.load(std::memory_order_acquire);
            Node *next = old_tail->next.load(std::memory_order_acquire);
            if (old_tail == tail.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    if (old_tail->next.compare_exchange_weak(
                            next, new_node, std::memory_order_release, std::memory_order_relaxed)) {
                        break;
                    }
                } else {
                    tail.compare_exchange_weak(old_tail, next, std::memory_order_release,
                                               std::memory_order_relaxed);
                }
            }
        }
        tail.compare_exchange_weak(old_tail, new_node, std::memory_order_release,
                                   std::memory_order_relaxed);
    }

    bool dequeue(T &value) {
        Node *old_head = nullptr;

        while (true) {
            old_head = head.load(std::memory_order_acquire);
            Node *old_tail = tail.load(std::memory_order_acquire);
            Node *next = old_head->next.load(std::memory_order_acquire);

            if (old_head == head.load(std::memory_order_acquire)) {
                if (old_head == old_tail) {
                    if (next == nullptr) {
                        return false;   // Queue is empty
                    }
                    tail.compare_exchange_weak(old_tail, next, std::memory_order_release,
                                               std::memory_order_relaxed);
                } else {
                    value = next->data;
                    if (head.compare_exchange_weak(old_head, next, std::memory_order_release,
                                                   std::memory_order_relaxed)) {
                        break;
                    }
                }
            }
        }

        delete old_head;
        return true;
    }

    // push, pop and is_empty wrapper
    void push(const T &value) { enqueue(value); }
    bool pop(T &value) { return dequeue(value); }
    bool is_empty() const {
        Node *first = head.load(std::memory_order_acquire);
        return (first == tail.load(std::memory_order_acquire) &&
                first->next.load(std::memory_order_acquire) == nullptr);
    }

  private:
    struct Node {
        T data;
        std::atomic<Node *> next;
        Node(T value) : data(value), next(nullptr) {}
    };

    std::atomic<Node *> head;
    std::atomic<Node *> tail;
};