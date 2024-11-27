
#include "HeavyPart.hpp"
#include "LightPart.hpp"
#include "frequency_estimator/FrequencyEstimatorBase.hpp"

template <int bucket_num, int tot_memory_in_bytes>
class ElasticSketch : public FrequencyEstimatorBase {
    static constexpr int heavy_mem = bucket_num * COUNTER_PER_BUCKET * 8;
    static constexpr int light_mem = tot_memory_in_bytes - heavy_mem;

    HeavyPart<bucket_num> heavy_part;
    LightPart<light_mem> light_part;

  public:
    ElasticSketch() {}
    ~ElasticSketch() {}
    void clear();

    void insert(uint8_t *key, int f = 1);
    void quick_insert(uint8_t *key, int f = 1);

    int query(uint8_t *key);
    int query_compressed_part(uint8_t *key, uint8_t *compress_part, int compress_counter_num);

    int get_compress_width(int ratio) { return light_part.get_compress_width(ratio); }
    void compress(int ratio, uint8_t *dst) { light_part.compress(ratio, dst); }

    int get_bucket_num() { return heavy_part.get_bucket_num(); }
    double get_bandwidth(int compress_ratio);

    void get_heavy_hitters(int threshold, vector<pair<string, int>> &results);
    int get_cardinality();
    double get_entropy();
    void get_distribution(vector<double> &dist);

    // interface for FrequencyEstimatorBase
    int total = 0;
    void update(const int &item, int c = 1) {
        this->total += c;
        insert((uint8_t *) &item, c);
    }
    void update(const std::string &item, int c = 1) {
        // what should be implemented here?
    }
    unsigned int estimate(const std::string &item) {
        // what should be implemented here?
        return 0;
    }
    unsigned int estimate(const int &item) { return query((uint8_t *) &item); }
    unsigned int update_and_estimate(const int &item, int c = 1) {
        update(item, c);
        return query((uint8_t *) &item);
    }
    unsigned int update_and_estimate(const std::string &item, int c = 1) {
        // what should be implemented here?
        return 0;
    }
    void print_status() {}

    void *operator new(size_t sz);
    void operator delete(void *p);
};