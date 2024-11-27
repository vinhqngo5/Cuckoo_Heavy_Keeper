#pragma once

#include "frequency_estimator/AugmentedSketch.hpp"
#include "frequency_estimator/BoundedKeyValuePriorityQueue.hpp"
#include "frequency_estimator/CountMinSketch.hpp"
#include "frequency_estimator/CuckooHeavyKeeper.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "frequency_estimator/HeapHashMapSpaceSaving.hpp"
#include "frequency_estimator/HeavyKeeper.hpp"
#include "frequency_estimator/MacroPreprocessor.hpp"
#include "frequency_estimator/OptimizedWeightedFrequent.hpp"
#include "frequency_estimator/SpaceSaving.hpp"
#include "frequency_estimator/StreamSummarySpaceSaving.hpp"
#include "frequency_estimator/WeightedFrequent.hpp"

template <typename T> struct FrequencyEstimatorTrait;

template <> struct FrequencyEstimatorTrait<CountMinConfig> {
    using type = CountMinSketch;
};
template <> struct FrequencyEstimatorTrait<AugmentedSketchConfig> {
    using type = AugmentedSketch;
};
template <> struct FrequencyEstimatorTrait<CuckooHeavyKeeperConfig> {
    using type = CuckooHeavyKeeper;
};
template <> struct FrequencyEstimatorTrait<HeavyKeeperConfig> {
    using type = HeavyKeeper;
};
template <> struct FrequencyEstimatorTrait<SpaceSavingConfig> {
    using type = HeapHashMapSpaceSavingV2;
};
template <> struct FrequencyEstimatorTrait<WeightedFrequentConfig> {
    using type = WeightedFrequent;
};
template <> struct FrequencyEstimatorTrait<OptimizedWeightedFrequentConfig> {
    using type = OptimizedWeightedFrequent;
};

template <typename T> struct FrequencyEstimatorConfigTrait;

template <> struct FrequencyEstimatorConfigTrait<CountMinSketch> {
    using type = CountMinConfig;
};

template <> struct FrequencyEstimatorConfigTrait<AugmentedSketch> {
    using type = AugmentedSketchConfig;
};

template <> struct FrequencyEstimatorConfigTrait<CuckooHeavyKeeper> {
    using type = CuckooHeavyKeeperConfig;
};

template <> struct FrequencyEstimatorConfigTrait<HeavyKeeper> {
    using type = HeavyKeeperConfig;
};

template <> struct FrequencyEstimatorConfigTrait<HeapHashMapSpaceSavingV2> {
    using type = SpaceSavingConfig;
};

template <> struct FrequencyEstimatorConfigTrait<WeightedFrequent> {
    using type = WeightedFrequentConfig;
};

template <> struct FrequencyEstimatorConfigTrait<OptimizedWeightedFrequent> {
    using type = OptimizedWeightedFrequentConfig;
};
