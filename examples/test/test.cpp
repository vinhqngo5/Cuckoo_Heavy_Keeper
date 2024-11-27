#include "frequency_estimator/FrequencyEstimatorFactory.hpp"

int main() {
    auto byte_size = 1000;

    auto heavy_keeper =
        Experiment1FrequencyEstimatorFactory::create_frequency_estimator<HeavyKeeper>(byte_size);
    auto cuckoo_heavy_keeper =
        Experiment1FrequencyEstimatorFactory::create_frequency_estimator<CuckooHeavyKeeper>(
            byte_size);
    auto count_min_sketch =
        Experiment1FrequencyEstimatorFactory::create_frequency_estimator<CountMinSketch>(byte_size);
    auto heap_hash_map_space_saving =
        Experiment1FrequencyEstimatorFactory::create_frequency_estimator<HeapHashMapSpaceSaving>(
            byte_size);
    auto space_saving =
        Experiment1FrequencyEstimatorFactory::create_frequency_estimator<SpaceSaving>(byte_size);

    return 0;
}