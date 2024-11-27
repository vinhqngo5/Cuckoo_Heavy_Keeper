#ifndef ALGORITHM
    // #define ALGORITHM count_min
    // #define ALGORITHM augmented_sketch
    // #define ALGORITHM heavy_keeper
    #define ALGORITHM cuckoo_heavy_keeper
#endif

#ifndef MODE
// #define MODE count
    #define MODE heavy_hitter
#endif
#include <iostream>

#include "delegation_sketch/DelegationBuildConfig.hpp"
#include "delegation_sketch/DelegationConfig.hpp"
#include "delegation_sketch/DelegationHeavyHitter.hpp"
#include "delegation_sketch/DelegationSketch.hpp"
#include "delegation_sketch/delegation_sketch_utils.hpp"
#include "frequency_estimator/AugmentedSketch.hpp"
#include "frequency_estimator/CountMinSketch.hpp"
#include "frequency_estimator/CuckooHeavyKeeper.hpp"
#include "frequency_estimator/FrequencyEstimatorConfig.hpp"
#include "frequency_estimator/MacroPreprocessor.hpp"
#include "heavy_hitter_app/AppConfig.hpp"

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
#endif

#if EQUAL(MODE, count)
using DelegationConfigBasedOnMode = DelegationConfig;
using MultithreadedApp = DelegationSketch<FrequencyEstimator>;
#elif EQUAL(MODE, heavy_hitter)
using DelegationConfigBasedOnMode = DelegationHeavyHitterConfig;
using MultithreadedApp = DelegationHeavyHitter<FrequencyEstimator>;
#endif

// assert that parser information is consistent
void assert_consistency(AppConfig &app_configs, DelegationConfigBasedOnMode &delegation_configs, FrequencyEstimatorConfig &frequency_estimator_configs);

int main(int argc, char **argv) {
    // init configs
    ConfigParser parser;

    // configs
    using AppConfig = ParallelAppConfig;
    AppConfig app_configs;
    DelegationConfigBasedOnMode delegation_configs;
    FrequencyEstimatorConfig frequency_estimator_configs;
    DelegationBuildConfig delegation_build_configs;

    // add parameters to the parser
    AppConfig::add_params_to_config_parser(app_configs, parser);
    FrequencyEstimatorConfig::add_params_to_config_parser(frequency_estimator_configs, parser);
    DelegationConfigBasedOnMode::add_params_to_config_parser(delegation_configs, parser);

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

    // print settings
    // print algorithm and mode
    PRINT_MACRO(ALGORITHM)
    PRINT_MACRO(MODE)

    // print important configs
    cout << app_configs << endl;
    cout << delegation_configs << endl;
    cout << frequency_estimator_configs << endl;

    // generate the dataset (relation)
    Relation *r1 = generate_relation(app_configs);

    // init vector of frequency_estimator objects
    vector<FrequencyEstimator> frequency_estimators;
    for (int i = 0; i < app_configs.NUM_THREADS; i++) { frequency_estimators.push_back(FrequencyEstimator(frequency_estimator_configs)); }

    // START_BENCHMARK flag
    atomic<bool> START_BENCHMARK = true;

    // start threads
    MultithreadedApp *delegation_sketch = start_threads<FrequencyEstimator>(app_configs, delegation_build_configs, delegation_configs, r1, frequency_estimators, START_BENCHMARK);

    // print stats
    print_stats_for_delegation_sketch(app_configs, delegation_sketch);
#if EQUAL(MODE, heavy_hitter)
    print_stats_for_heavy_hitters<FrequencyEstimator, int>(app_configs, delegation_build_configs, delegation_configs, delegation_sketch, r1);
#endif

    return 0;
}
