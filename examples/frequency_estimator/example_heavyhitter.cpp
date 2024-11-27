#ifndef ALGORITHM
    #define ALGORITHM count_min
// #define ALGORITHM augmented_sketch
// #define ALGORITHM heavy_keeper
// #define ALGORITHM cuckoo_heavy_keeper
// #define ALGORITHM weighted_frequent
#endif

#include "delegation_sketch/DelegationConfig.hpp"
#include "frequency_estimator/FrequencyEstimatorTrait.hpp"
#include "frequency_estimator/SequentialHeavyHitterWrapper.hpp"
#include "frequency_estimator/sequential_heavyhitter_test_utils.hpp"
#include "heavy_hitter_app/AppConfig.hpp"

#include <iostream>

using namespace std;

#if EQUAL(ALGORITHM, count_min)
using FrequencyEstimatorConfig = CountMinConfig;
using FrequencyEstimator = CountMinSketch;
#elif EQUAL(ALGORITHM, augmented_sketch)
using FrequencyEstimatorConfig = AugmentedSketchConfig;
using FrequencyEstimator = AugmentedSketch;
#elif EQUAL(ALGORITHM, cuckoo_heavy_keeper)
using FrequencyEstimatorConfig = CuckooHeavyKeeperConfig;
using FrequencyEstimator = CuckooHeavyKeeper;
#elif EQUAL(ALGORITHM, heavy_keeper)
using FrequencyEstimatorConfig = HeavyKeeperConfig;
using FrequencyEstimator = HeavyKeeper;
#elif EQUAL(ALGORITHM, simple_space_saving)
using FrequencyEstimatorConfig = SpaceSavingConfig;
using FrequencyEstimator = SpaceSaving;
#elif EQUAL(ALGORITHM, stream_summary_space_saving)
using FrequencyEstimatorConfig = SpaceSavingConfig;
using FrequencyEstimator = StreamSummarySpaceSaving;
#elif EQUAL(ALGORITHM, heap_hashmap_space_saving)
using FrequencyEstimatorConfig = SpaceSavingConfig;
using FrequencyEstimator = HeapHashMapSpaceSaving;
#elif EQUAL(ALGORITHM, weighted_frequent)
using FrequencyEstimatorConfig = WeightedFrequentConfig;
using FrequencyEstimator = WeightedFrequent;
#elif EQUAL(ALGORITHM, optimized_weighted_frequent)
using FrequencyEstimatorConfig = OptimizedWeightedFrequentConfig;
using FrequencyEstimator = OptimizedWeightedFrequent;
#else
#endif

// configs
using AppConfig = SequentialAppConfig;
AppConfig app_configs;
FrequencyEstimatorConfig frequency_estimator_configs;

int main(int argc, char **argv) {
    // init configs
    ConfigParser parser;

    // add parameters to the parser
    AppConfig::add_params_to_config_parser(app_configs, parser);
    FrequencyEstimatorConfig::add_params_to_config_parser(frequency_estimator_configs, parser);

    // run the parser
    if (argc == 2 && (strncmp(argv[1], "--help", 6) == 0 || strncmp(argv[1], "-h", 2) == 0)) {
        parser.PrintUsage();
        exit(0);
    }

    if (argc == 2 && (strncmp(argv[1], "--generate-doc", 6) == 0 || strncmp(argv[1], "-h", 2) == 0)) {
        fprintf(stdout, "Generating the parameter list in markdown format for use in the documentation.\n\n");
        parser.PrintMarkdown();
        exit(0);
    }

    Status s = parser.ParseCommandLine(argc, argv);
    if (!s.IsOK()) {
        fprintf(stderr, "%s\n", s.ToString().c_str());
        exit(-1);
    }

    // print the configs
    cout << frequency_estimator_configs;
    cout << app_configs;

    // test
    test(frequency_estimator_configs, app_configs);
}
