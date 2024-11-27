#include "frequency_estimator/CountMinSketch.hpp"
#include "frequency_estimator/CuckooHeavyKeeper.hpp"
#include "frequency_estimator/FrequencyEstimatorBase.hpp"
#include "frequency_estimator/HeapHashMapSpaceSaving.hpp"
#include "frequency_estimator/HeavyKeeper.hpp"
#include "frequency_estimator/SpaceSaving.hpp"
#include "frequency_estimator/StreamSummarySpaceSaving.hpp"

class Experiment1FrequencyEstimatorFactory {
  public:
    template <typename T, typename... Args>
        requires std::derived_from<T, FrequencyEstimatorBase>
    static T create_frequency_estimator(const int byte_size, Args &&...args) {
        if constexpr (std::same_as<T, HeavyKeeper>) {
            return Experiment1FrequencyEstimatorFactory::create_HeavyKeeper(byte_size);
        } else if constexpr (std::same_as<T, CuckooHeavyKeeper>) {
            return Experiment1FrequencyEstimatorFactory::create_CuckooHeavyKeeper(byte_size);
        } else if constexpr (std::same_as<T, CountMinSketch>) {
            return Experiment1FrequencyEstimatorFactory::create_CountMinSketch(byte_size);
        } else if constexpr (std::same_as<T, HeapHashMapSpaceSaving>) {
            return Experiment1FrequencyEstimatorFactory::create_HeapHashMapSpaceSaving(byte_size);
        } else if constexpr (std::same_as<T, SpaceSaving>) {
            return Experiment1FrequencyEstimatorFactory::create_SpaceSaving(byte_size);
        } else {
            static_assert(std::false_type::value, "Unsupported type");
        }
    }

  private:
    static HeavyKeeper create_HeavyKeeper(const int byte_size) {
        int num_buckets = byte_size / sizeof(HeavyKeeper::node) / HeavyKeeper::HK_d;
        return HeavyKeeper(num_buckets);
    }
    static CuckooHeavyKeeper create_CuckooHeavyKeeper(const int byte_size) {
        int bucket_size = (sizeof(CuckooHeavyKeeper::bucket_t) - 3);
        int num_buckets = byte_size / bucket_size / 2;
        return CuckooHeavyKeeper(num_buckets, 0.001);
    }

    static CountMinSketch create_CountMinSketch(const int byte_size) {
        unsigned int depth = 8;
        unsigned int width = byte_size / sizeof(int) / depth;
        return CountMinSketch(width, depth);
    }

    static HeapHashMapSpaceSaving create_HeapHashMapSpaceSaving(const int byte_size) {
        int num_buckets = byte_size / (sizeof(HeapHashMapSpaceSaving::Item) * 2);
        return HeapHashMapSpaceSaving(num_buckets);
    }

    static SpaceSaving create_SpaceSaving(const int byte_size) {
        int num_buckets = byte_size / (sizeof(std::string) + sizeof(unsigned int));
        return SpaceSaving(num_buckets);
    }
};
