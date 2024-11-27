#pragma once

#include "delegation_sketch/DelegationConfig.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "frequency_estimator/FrequencyEstimatorTrait.hpp"
#include "prif/PRIFConfig.hpp"
#include <mutex>
#include <queue>
#include <semaphore>
#include <unordered_map>
#include <variant>

enum class MessageType { Update, Query, STOP };

template <typename KeyType> struct Message {
    MessageType type;
    KeyType item;
    int delta;
};

template <typename KeyType> struct QueryResult {
    KeyType item;
    int frequency;
};

template <typename KeyType> struct SharedBuffer {
    std::queue<Message<KeyType>> buffer;
    std::counting_semaphore<> semmesgs;
    std::counting_semaphore<> semslots;
    std::mutex buffer_mutex;

    std::queue<QueryResult<KeyType>> query_results;
    std::counting_semaphore<> semqueries;
    std::mutex query_mutex;

    SharedBuffer(int capacity) : semmesgs(0), semslots(capacity), semqueries(1) {}
};

template <typename FrequencyEstimator, typename KeyType> class ThreadLocalPRIF {
  public:
    FrequencyEstimator frequency_estimator;

    using FrequencyEstimatorConfig = typename FrequencyEstimatorConfigTrait<FrequencyEstimator>::type;
    FrequencyEstimatorConfig frequency_estimator_configs;
    int beta;
    int total;
    int current_thread_id;
    std::unordered_map<KeyType, int> delta;
    SharedBuffer<KeyType> *shared_buffer;

    ThreadLocalPRIF() = default;
    ThreadLocalPRIF(FrequencyEstimatorConfig frequency_estimator_configs) : frequency_estimator_configs(frequency_estimator_configs) {
        frequency_estimator = FrequencyEstimator(frequency_estimator_configs);
    }

    // Prevent copying
    ThreadLocalPRIF(const ThreadLocalPRIF &) = delete;
    ThreadLocalPRIF &operator=(const ThreadLocalPRIF &) = delete;

    // allowing moving
    ThreadLocalPRIF(ThreadLocalPRIF &&) = default;
    ThreadLocalPRIF &operator=(ThreadLocalPRIF &&) = default;

    void update(const KeyType &item, int c = 1) {
        this->total += c;

        delta[item] += c;
        int freq = frequency_estimator.update_and_estimate(item, c);

        if (freq > beta * total) {
            _send_message(item, delta[item]);
            delta[item] = 0;   // Reset the frequency increment field
        }
    }

    void flush() {
        for (auto &entry : delta) { _send_message(entry.first, entry.second); }
        delta.clear();
    }

  private:
    void _send_message(const KeyType &item, int delta_value) {
        Message<KeyType> msg{MessageType::Update, item, delta_value};
        shared_buffer->semslots.acquire();
        {
            std::lock_guard<std::mutex> lock(shared_buffer->buffer_mutex);
            shared_buffer->buffer.push(msg);
        }
        shared_buffer->semmesgs.release();
    }
};

template <typename FrequencyEstimator, typename KeyType> class MergingThreadPRIF {
  public:
    using FrequencyEstimatorConfig = typename FrequencyEstimatorConfigTrait<FrequencyEstimator>::type;
    FrequencyEstimator frequency_estimator;
    FrequencyEstimatorConfig frequency_estimator_configs;
    SharedBuffer<KeyType> *shared_buffer;

    MergingThreadPRIF() = default;
    MergingThreadPRIF(FrequencyEstimatorConfig frequency_estimator_configs) : frequency_estimator_configs(frequency_estimator_configs) {
        frequency_estimator = FrequencyEstimator(frequency_estimator_configs);
    }

    void run() {
        while (true) {
            Message<KeyType> msg = _receive_message();
            if (msg.type == MessageType::Update) {
                frequency_estimator.update(msg.item, msg.delta);
            } else if (msg.type == MessageType::Query) {
                int frequency = frequency_estimator.estimate(msg.item);
                _send_query_result(msg.item, frequency);
            } else if (msg.type == MessageType::STOP) {
                break;
            }
        }
    }

  private:
    Message<KeyType> _receive_message() {
        shared_buffer->semmesgs.acquire();
        Message<KeyType> msg;
        {
            std::lock_guard<std::mutex> lock(shared_buffer->buffer_mutex);
            msg = shared_buffer->buffer.front();
            shared_buffer->buffer.pop();
        }
        shared_buffer->semslots.release();
        return msg;
    }

    void _send_query_result(const KeyType &item, int frequency) {
        QueryResult<KeyType> result{item, frequency};
        {
            std::lock_guard<std::mutex> lock(shared_buffer->query_mutex);
            shared_buffer->query_results.push(result);
        }
        shared_buffer->semqueries.release();
    }
};

template <typename FrequencyEstimator, typename KeyType> class PRIF {
  public:
    PRIFConfig prif_configs;
    using FrequencyEstimatorConfig = typename FrequencyEstimatorConfigTrait<FrequencyEstimator>::type;
    FrequencyEstimatorConfig frequency_estimator_configs;
    int num_threads;
    int beta;
    int total;
    std::vector<ThreadLocalPRIF<FrequencyEstimator, KeyType>> thread_local_prifs;
    MergingThreadPRIF<FrequencyEstimator, KeyType> merging_thread_prif;
    SharedBuffer<KeyType> shared_buffer;

    PRIF(PRIFConfig prif_configs, FrequencyEstimatorConfig frequency_estimator_configs)
        : prif_configs(prif_configs), num_threads(prif_configs.NUM_THREADS), beta(prif_configs.BETA), shared_buffer(num_threads),
          frequency_estimator_configs(frequency_estimator_configs) {
        this->total = 0;

        merging_thread_prif = MergingThreadPRIF<FrequencyEstimator, KeyType>(frequency_estimator_configs);

        for (int i = 0; i < num_threads; i++) {
            thread_local_prifs.emplace_back(frequency_estimator_configs);
            thread_local_prifs[i].beta = this->beta;
            thread_local_prifs[i].current_thread_id = i;
            thread_local_prifs[i].shared_buffer = &this->shared_buffer;
        }
        merging_thread_prif.shared_buffer = &this->shared_buffer;
    }

    void send_stop_to_merging_thread() {
        Message<KeyType> msg{MessageType::STOP, KeyType(), 0};
        shared_buffer.semslots.acquire();
        {
            std::lock_guard<std::mutex> lock(shared_buffer.buffer_mutex);
            shared_buffer.buffer.push(msg);
        }
        shared_buffer.semmesgs.release();
    }

    int query(const KeyType &item) {
        _send_query(item);
        QueryResult<KeyType> result = _receive_query_result();
        return result.frequency;
    }

  private:
    void _send_query(const KeyType &item) {
        Message<KeyType> msg{MessageType::Query, item, 0};
        shared_buffer.semslots.acquire();
        {
            std::lock_guard<std::mutex> lock(shared_buffer.buffer_mutex);
            shared_buffer.buffer.push(msg);
        }
        shared_buffer.semmesgs.release();
    }

    QueryResult<KeyType> _receive_query_result() {
        shared_buffer.semqueries.acquire();
        QueryResult<KeyType> result;
        {
            std::lock_guard<std::mutex> lock(shared_buffer.query_mutex);
            result = shared_buffer.query_results.front();
            shared_buffer.query_results.pop();
        }
        return result;
    }
};