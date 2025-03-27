#pragma once

#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "frequency_estimator/MacroPreprocessor.hpp"
#include "hash/BOBHash32.hpp"
#include "hash/BOBHash64.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// using fingerprint_t = uint16_t;
// using counter_t = uint32_t;
using fingerprint_t = int;
using counter_t = int;

class CuckooHeavyKeeper : public FrequencyEstimatorBase {
  public:
    struct Entry {
        fingerprint_t fingerprint{0};
        counter_t counter{0};
        bool is_empty() const { return counter == 0; }
    };

    struct Bucket {
        static constexpr size_t ENTRIES_PER_BUCKET = 3;
        Entry entries[ENTRIES_PER_BUCKET];   // First entry is lobby

        Entry &get_lobby() { return entries[0]; }
        const Entry &get_lobby() const { return entries[0]; }

        Entry &get_smallest_heavy() {
            Entry *smallest = &entries[1];
            for (size_t i = 2; i < ENTRIES_PER_BUCKET; ++i) {
                if (entries[i].counter < smallest->counter) { smallest = &entries[i]; }
            }
            return *smallest;
        }
    };

    explicit CuckooHeavyKeeper(size_t bucket_num = 128, double theta = 0.01, counter_t promotion_threshold = 16, double decay_base = 1.08);
    explicit CuckooHeavyKeeper(CuckooHeavyKeeperConfig config);

    ~CuckooHeavyKeeper();

    size_t total{0};
    void update(const int &item, int c = 1) override;
    void update(const std::string &item, int c = 1) override;
    unsigned int estimate(const int &item) override;
    unsigned int estimate(const std::string &item) override;
    unsigned int update_and_estimate(const int &item, int c = 1) override;
    unsigned int update_and_estimate(const std::string &item, int c = 1) override;
    void print_status() override;

    friend std::ostream &operator<<(std::ostream &os, const CuckooHeavyKeeper &ck);

  private:
    static constexpr size_t MAX_KICKS = 10;
    static constexpr size_t FINGERPRINT_BITS = 16;
    static constexpr size_t MAX_COUNTER = 16;
    static constexpr double HEAVY_RATIO = 0.8;

    size_t m_bucket_num;
    counter_t m_promotion_threshold;
    double m_decay_base;
    double m_theta;

    std::array<std::vector<Bucket>, 2> m_tables;
    BOBHash64 *m_bobhash;
    std::array<double, MAX_COUNTER + 1> m_decay_expectations;
    std::array<double, MAX_COUNTER + 1> m_min_decay_amounts;
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> dist;

    bool _is_power_of_two(size_t x) const { return x && !(x & (x - 1)); }
    void _init_decay_expectations();

    void _generate_fingerprint_and_index(const std::string &item, fingerprint_t &fp, size_t &idx) const;
    size_t _generate_alt_index(fingerprint_t fp, size_t idx) const;
    unsigned long long _hash(const std::string &item) const;

    counter_t _update_impl(const std::string &item, int weight);
    counter_t _estimate_impl(const std::string &item) const;

    bool _check_and_update_heavy(fingerprint_t fp, size_t idx1, size_t idx2, int weight, counter_t &result);
    bool _check_and_update_lobby(fingerprint_t fp, size_t idx1, size_t idx2, int weight, counter_t &result);
    counter_t _decay_counter(counter_t current, int weight);

    bool _try_promote_and_kickout(Entry &lobby, Entry &smallest, size_t table_idx, size_t idx);
    void _do_kickout(Entry kicked, size_t curr_table_idx, size_t curr_idx);

    bool _is_heavy_hitter(counter_t count) const { return count >= (total * m_theta * HEAVY_RATIO); }
};

std::ostream &operator<<(std::ostream &os, const CuckooHeavyKeeper &ck);
