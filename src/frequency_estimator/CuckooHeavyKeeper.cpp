#include "CuckooHeavyKeeper.hpp"
#include <cassert>
#include <ctime>
#include <iomanip>

CuckooHeavyKeeper::CuckooHeavyKeeper(size_t bucket_num, double theta, counter_t promotion_threshold, double decay_base)
    : m_bucket_num(bucket_num), m_theta(theta), m_promotion_threshold(promotion_threshold), m_decay_base(decay_base) {
    assert(_is_power_of_two(bucket_num) && "bucket_num must be power of 2");

    m_tables[0].resize(bucket_num);
    m_tables[1].resize(bucket_num);

    srand(static_cast<unsigned int>(clock()));
    m_bobhash = new BOBHash64(rand() % 1228);
    _init_decay_expectations();
}

CuckooHeavyKeeper::CuckooHeavyKeeper(CuckooHeavyKeeperConfig config) : CuckooHeavyKeeper(config.BUCKET_NUM, config.THETA, 16, 1.08) {}

CuckooHeavyKeeper::~CuckooHeavyKeeper() {
    // delete m_bobhash;
}

void CuckooHeavyKeeper::_init_decay_expectations() {
    m_decay_expectations[0] = 0;
    for (int i = 1; i <= MAX_COUNTER; i++) { m_decay_expectations[i] = m_decay_expectations[i - 1] + std::pow(m_decay_base, i - 1); }
}

void CuckooHeavyKeeper::_generate_fingerprint_and_index(const std::string &item, fingerprint_t &fp, size_t &idx) const {
    unsigned long long h = _hash(item);
    fp = h & ((1ULL << FINGERPRINT_BITS) - 1);
    idx = (h >> 32) & (m_bucket_num - 1);
}

size_t CuckooHeavyKeeper::_generate_alt_index(fingerprint_t fp, size_t idx) const { return (idx ^ (0x5bd1e995 * fp)) & (m_bucket_num - 1); }

unsigned long long CuckooHeavyKeeper::_hash(const std::string &item) const { return m_bobhash->run(item.c_str(), item.size()); }

void CuckooHeavyKeeper::_do_kickout(Entry kicked, size_t curr_table_idx, size_t curr_idx) {
    size_t kicks = 0;
    while (kicks < MAX_KICKS) {
        if (!_is_heavy_hitter(kicked.counter)) { return; }

        curr_table_idx = 1 - curr_table_idx;
        curr_idx = _generate_alt_index(kicked.fingerprint, curr_idx);

        Bucket &curr_bucket = m_tables[curr_table_idx][curr_idx];
        Entry &curr_smallest = curr_bucket.get_smallest_heavy();

        if (curr_smallest.is_empty()) {
            curr_smallest = kicked;
            return;
        }

        std::swap(curr_smallest, kicked);
        kicks++;
    }
}

bool CuckooHeavyKeeper::_try_promote_and_kickout(Entry &lobby, Entry &target, size_t table_idx, size_t idx) {
    // Handle empty target case
    if (target.is_empty()) {
        std::swap(target, lobby);
        return true;
    }

    // Calculate promotion probability only if target counter is greater
    if (target.counter > lobby.counter) {
        double prob = (lobby.counter - m_promotion_threshold) * (1.0 / (target.counter - m_promotion_threshold));
        if ((static_cast<double>(rand()) / RAND_MAX) >= prob) { return false; }
    }

    // Handle promotion and kickout
    Entry kicked = target;
    std::swap(target, lobby);
    lobby = Entry{};
    _do_kickout(kicked, table_idx, idx);
    return true;
}

counter_t CuckooHeavyKeeper::_decay_counter(counter_t current, int weight) {
    if (current == 0) return 0;

    if (weight == 1) {
        // Original Heavy Keeper decay with probability b^(-current)
        double decay_prob = std::pow(m_decay_base, -current);
        if ((static_cast<double>(rand()) / RAND_MAX) < decay_prob) { return current - 1; }
        return current;
    }

    if (weight >= m_decay_expectations[current]) return 0;

    int left = 0;
    int right = current;

    // Binary search to find first position where m_decay_expectations[idx] + weight >= m_decay_expectations[current]
    //<=> m_decay_expectations[idx]  >= m_decay_expectations[current] - weight

    while (left < right) {
        int mid = left + (right - left) / 2;

        if (m_decay_expectations[mid] + weight >= m_decay_expectations[current]) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    return left;
}

bool CuckooHeavyKeeper::_check_and_update_heavy(fingerprint_t fp, size_t idx1, size_t idx2, int weight, counter_t &result) {
    size_t empty_table_idx = -1;
    size_t empty_pos = -1;

    // Check if exists in heavy entries and track first empty slot
    for (size_t table_idx = 0; table_idx < 2; ++table_idx) {
        size_t idx = (table_idx == 0) ? idx1 : idx2;
        Bucket &bucket = m_tables[table_idx][idx];

        for (size_t i = 1; i < Bucket::ENTRIES_PER_BUCKET; ++i) {
            Entry &entry = bucket.entries[i];
            if (entry.counter > 0) {
                if (entry.fingerprint == fp) {
                    entry.counter += weight;
                    result = entry.counter;
                    return true;
                }
            } else if (empty_table_idx == -1) {
                empty_table_idx = table_idx;
                empty_pos = i;
            }
        }
    }

    // If not found but we have empty slot -> just put it there -> less collision
    if (empty_table_idx != -1) {
        size_t idx = (empty_table_idx == 0) ? idx1 : idx2;
        m_tables[empty_table_idx][idx].entries[empty_pos] = Entry{fp, weight};
        result = weight;
        return true;
    }

    return false;
}

bool CuckooHeavyKeeper::_check_and_update_lobby(fingerprint_t fp, size_t idx1, size_t idx2, int weight, counter_t &result) {
    for (size_t table_idx = 0; table_idx < 2; ++table_idx) {
        size_t idx = (table_idx == 0) ? idx1 : idx2;
        Bucket &bucket = m_tables[table_idx][idx];
        Entry &lobby = bucket.get_lobby();

        if (lobby.counter > 0 && lobby.fingerprint == fp) {
            lobby.counter += weight;

            if (lobby.counter >= m_promotion_threshold) {
                Entry &smallest = bucket.get_smallest_heavy();
                if (_try_promote_and_kickout(lobby, smallest, table_idx, idx)) {
                    result = smallest.counter;
                    return true;
                }
                lobby.counter = m_promotion_threshold;
            }
            result = lobby.counter;
            return true;
        }
    }
    return false;
}

counter_t CuckooHeavyKeeper::_update_impl(const std::string &item, int weight) {
    total += weight;
    fingerprint_t fp;
    size_t idx1;
    _generate_fingerprint_and_index(item, fp, idx1);
    size_t idx2 = _generate_alt_index(fp, idx1);

    counter_t result;

    // check if it is in heavy entries first -> update and return
    if (_check_and_update_heavy(fp, idx1, idx2, weight, result)) { return result; }

    // check if it is in lobby entries -> update and return
    if (_check_and_update_lobby(fp, idx1, idx2, weight, result)) { return result; }

    size_t target_table_idx = 0;
    size_t target_idx = 0;
    bool found_empty = false;

    // check for empty lobby entries first
    for (size_t table_idx = 0; table_idx < 2; ++table_idx) {
        size_t idx = (table_idx == 0) ? idx1 : idx2;
        Entry &lobby = m_tables[table_idx][idx].get_lobby();

        if (lobby.is_empty()) {
            target_table_idx = table_idx;
            target_idx = idx;
            lobby = Entry{fp, weight};
            found_empty = true;
            break;
        }
    }

    // if no empty entries found, use consistent table selection
    if (!found_empty) {
        target_table_idx = fp & 1;
        target_idx = (target_table_idx == 0) ? idx1 : idx2;
    }

    Entry &target_lobby = m_tables[target_table_idx][target_idx].get_lobby();

    if (!found_empty) {
        counter_t new_count = _decay_counter(target_lobby.counter, weight);
        target_lobby = (new_count == 0) ? Entry{fp, weight - m_decay_expectations[target_lobby.counter]} : Entry{target_lobby.fingerprint, new_count};
    }

    if (target_lobby.counter >= m_promotion_threshold) {
        Entry &smallest = m_tables[target_table_idx][target_idx].get_smallest_heavy();
        if (_try_promote_and_kickout(target_lobby, smallest, target_table_idx, target_idx)) { return smallest.counter; }
        target_lobby.counter = m_promotion_threshold;
    }

    return target_lobby.counter;
}

counter_t CuckooHeavyKeeper::_estimate_impl(const std::string &item) const {
    fingerprint_t fp;
    size_t idx1;
    _generate_fingerprint_and_index(item, fp, idx1);
    size_t idx2 = _generate_alt_index(fp, idx1);

    counter_t max_count = 0;
    for (size_t table_idx = 0; table_idx < 2; ++table_idx) {
        size_t idx = (table_idx == 0) ? idx1 : idx2;
        const Bucket &bucket = m_tables[table_idx][idx];

        for (const auto &entry : bucket.entries) {
            if (entry.counter > 0 && entry.fingerprint == fp) { max_count = std::max(max_count, entry.counter); }
        }
    }
    return max_count;
}

void CuckooHeavyKeeper::update(const int &item, int c) { update(std::to_string(item), c); }

void CuckooHeavyKeeper::update(const std::string &item, int c) { _update_impl(item, c); }

unsigned int CuckooHeavyKeeper::estimate(const int &item) { return estimate(std::to_string(item)); }

unsigned int CuckooHeavyKeeper::estimate(const std::string &item) { return _estimate_impl(item); }

unsigned int CuckooHeavyKeeper::update_and_estimate(const int &item, int c) { return update_and_estimate(std::to_string(item), c); }

unsigned int CuckooHeavyKeeper::update_and_estimate(const std::string &item, int c) { return _update_impl(item, c); }

void CuckooHeavyKeeper::print_status() { std::cout << *this << std::endl; }

std::ostream &operator<<(std::ostream &os, const CuckooHeavyKeeper &ck) {
    os << "CuckooHeavyKeeper Status:\n"
       << "Bucket Number: " << ck.m_bucket_num << "\n"
       << "Promotion Threshold: " << ck.m_promotion_threshold << "\n"
       << "Decay Base: " << ck.m_decay_base << "\n"
       << "Total Items: " << ck.total << "\n\n";

    // Helper for drawing horizontal line
    auto draw_line = [&os](size_t width) { os << '+' << std::string(width * 4, '-') << "+\n"; };

    // Print each table
    for (size_t table_idx = 0; table_idx < 2; ++table_idx) {
        os << "Table " << table_idx << ":\n";

        for (size_t bucket_idx = 0; bucket_idx < ck.m_bucket_num; ++bucket_idx) {
            const auto &bucket = ck.m_tables[table_idx][bucket_idx];

            draw_line(15);   // Width of each cell is 15
            os << '|';

            // Print each entry in the bucket
            for (const auto &entry : bucket.entries) { os << std::setw(7) << entry.fingerprint << "," << std::setw(6) << entry.counter << " |"; }
            os << '\n';
        }
        draw_line(15);
        os << '\n';
    }

    return os;
}
