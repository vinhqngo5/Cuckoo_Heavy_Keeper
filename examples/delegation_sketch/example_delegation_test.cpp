
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
using MultithreadedApp = DelegationSketch<FrequencyEstimator>;
#endif

// configs
using AppConfig = ParallelAppConfig;
AppConfig app_configs;
DelegationConfigBasedOnMode delegation_configs;
FrequencyEstimatorConfig frequency_estimator_configs;
DelegationBuildConfig delegation_build_configs;

int main(int argc, char **argv) {
    // init configs
    ConfigParser parser;

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

    // print important configs
    cout << delegation_build_configs << endl;
    cout << app_configs << endl;
    cout << delegation_configs << endl;
    cout << frequency_estimator_configs << endl;

    if constexpr (delegation_build_configs.mode == "count") {
        cout << "count" << endl;
    } else if constexpr (delegation_build_configs.mode == "heavy_hitter") {
        cout << "heavy_hitter" << endl;
    }
}