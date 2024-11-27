#pragma once

#include "utils/ConfigParser.hpp"
#include "utils/ConfigPrinter.hpp"
#include <getopt.h>
#include <string>

using namespace std;

struct CountMinConfig {
    unsigned int WIDTH;
    unsigned int DEPTH;
    float DELTA;
    float EPSILON;
    string CALCULATE_FROM;   // CALCULATE_FROM WIDTH_DEPTH or EPSILON_DELTA

    static void add_params_to_config_parser(CountMinConfig &countmin_config, ConfigParser &parser) {
        // CountMin configs prefix will be "countmin."
        parser.AddParameter(new UnsignedInt32Parameter("countmin.width", "128", &countmin_config.WIDTH, false, "Width of the count min sketch"));
        parser.AddParameter(new UnsignedInt32Parameter("countmin.depth", "8", &countmin_config.DEPTH, false, "Depth of the count min sketch"));
        parser.AddParameter(new FloatParameter("countmin.epsilon", "0.01", &countmin_config.EPSILON, false, "Epsilon value for the count min sketch"));
        parser.AddParameter(new FloatParameter("countmin.delta", "0.01", &countmin_config.DELTA, false, "Delta value for the count min sketch"));
        parser.AddParameter(new StringParameter("countmin.calculate_from", "WIDTH_DEPTH", &countmin_config.CALCULATE_FROM, false,
                                                "Calculate the count min sketch from "
                                                "WIDTH_DEPTH or EPSILON_DELTA"));
    }

    auto to_tuple() const { return std::make_tuple("WIDTH", WIDTH, "DEPTH", DEPTH, "DELTA", DELTA, "EPSILON", EPSILON, "CALCULATE_FROM", CALCULATE_FROM); }

    friend std::ostream &operator<<(std::ostream &os, const CountMinConfig &config) {
        ConfigPrinter<CountMinConfig>::print(os, config);
        return os;
    }
};

struct HeavyKeeperConfig {
    int K;
    int M2;
    static void add_params_to_config_parser(HeavyKeeperConfig &heavykeeper_config, ConfigParser &parser) {
        // HeavyKeeper configs prefix will be "heavykeeper."
        parser.AddParameter(new IntParameter("heavykeeper.k", "100", &heavykeeper_config.K, false, "Number of heavy hitters to keep"));
        parser.AddParameter(new IntParameter("heavykeeper.m2", "100", &heavykeeper_config.M2, false, "Number of buckets"));
    }

    auto to_tuple() const { return std::make_tuple("K", K, "M2", M2); }
    friend std::ostream &operator<<(std::ostream &os, const HeavyKeeperConfig &config) {
        ConfigPrinter<HeavyKeeperConfig>::print(os, config);
        return os;
    }
};
struct CuckooHeavyKeeperConfig {
    int BUCKET_NUM;
    int MAX_LOOP;
    int BITS_PER_ITEM;
    float THETA;
    float HK_b;
    // string CALCULATE_WORTH_REINSERTING_FROM; // total,passed_total
    // this is set in CuckooHeavyKeeper macro as it will be fixed
    // int NUM_ENTRY_PER_BUCKET;
    static void add_params_to_config_parser(CuckooHeavyKeeperConfig &cuckooheavykeeper_config, ConfigParser &parser) {
        // CuckooHeavyKeeper configs prefix will be "cuckooheavykeeper."
        parser.AddParameter(new IntParameter("cuckooheavykeeper.bucket_num", "1024", &cuckooheavykeeper_config.BUCKET_NUM, false, "Number of buckets"));
        parser.AddParameter(new IntParameter("cuckooheavykeeper.max_loop", "10", &cuckooheavykeeper_config.MAX_LOOP, false, "Maximum loop for cuckoo hashing"));
        parser.AddParameter(new IntParameter("cuckooheavykeeper.bits_per_item", "16", &cuckooheavykeeper_config.BITS_PER_ITEM, false, "Number of bits per item"));
        // parser.AddParameter(new
        // IntParameter("cuckooheavykeeper.num_entry_per_bucket", "4",
        // &cuckooheavykeeper_config.NUM_ENTRY_PER_BUCKET, false, "Number of
        // entries per bucket"));
        parser.AddParameter(new FloatParameter("cuckooheavykeeper.theta", "0.1", &cuckooheavykeeper_config.THETA, false, "Theta value for the cuckoo heavy keeper"));
        parser.AddParameter(new FloatParameter("cuckooheavykeeper.HK_b", "1.08", &cuckooheavykeeper_config.HK_b, false, "b value for the cuckoo heavy keeper"));
        // parser.AddParameter(new
        // StringParameter("cuckooheavykeeper.calculate_worth_reinserting_from",
        // "total", &cuckooheavykeeper_config.CALCULATE_WORTH_REINSERTING_FROM,
        // false, "Calculate worth reinserting from total or passed_total"));
    }

    auto to_tuple() const { return std::make_tuple("BUCKET_NUM", BUCKET_NUM, "MAX_LOOP", MAX_LOOP, "BITS_PER_ITEM", BITS_PER_ITEM, "THETA", THETA, "HK_b", HK_b); }

    friend std::ostream &operator<<(std::ostream &os, const CuckooHeavyKeeperConfig &config) {
        ConfigPrinter<CuckooHeavyKeeperConfig>::print(os, config);
        return os;
    }
};

struct AugmentedSketchConfig {
    int FILTER_SIZE;
    unsigned int WIDTH;
    unsigned int DEPTH;
    float DELTA;
    float EPSILON;
    string CALCULATE_FROM;   // CALCULATE_FROM WIDTH_DEPTH or EPSILON_DELTA
    static void add_params_to_config_parser(AugmentedSketchConfig &augmentedsketch_config, ConfigParser &parser) {
        // AugmentedSketch configs prefix will be "augmentedsketch."
        parser.AddParameter(new IntParameter("augmentedsketch.filter_size", "16", &augmentedsketch_config.FILTER_SIZE, false, "Filter size for the augmented sketch"));
        parser.AddParameter(new UnsignedInt32Parameter("augmentedsketch.width", "128", &augmentedsketch_config.WIDTH, false, "Width of the augmented sketch"));
        parser.AddParameter(new UnsignedInt32Parameter("augmentedsketch.depth", "8", &augmentedsketch_config.DEPTH, false, "Depth of the augmented sketch"));
        parser.AddParameter(new FloatParameter("augmentedsketch.epsilon", "0.01", &augmentedsketch_config.EPSILON, false, "Epsilon value for the augmented sketch"));
        parser.AddParameter(new FloatParameter("augmentedsketch.delta", "0.01", &augmentedsketch_config.DELTA, false, "Delta value for the augmented sketch"));
        parser.AddParameter(new StringParameter("augmentedsketch.calculate_from", "WIDTH_DEPTH", &augmentedsketch_config.CALCULATE_FROM, false,
                                                "Calculate the augmented sketch from "
                                                "WIDTH_DEPTH or EPSILON_DELTA"));
    }

    // cast from AugmentedSketchConfig to CountMinConfig
    operator CountMinConfig() const {
        CountMinConfig countmin_config;
        countmin_config.WIDTH = this->WIDTH;
        countmin_config.DEPTH = this->DEPTH;
        countmin_config.DELTA = this->DELTA;
        countmin_config.EPSILON = this->EPSILON;
        countmin_config.CALCULATE_FROM = this->CALCULATE_FROM;
        return countmin_config;
    }

    auto to_tuple() const {
        return std::make_tuple("FILTER_SIZE", FILTER_SIZE, "WIDTH", WIDTH, "DEPTH", DEPTH, "DELTA", DELTA, "EPSILON", EPSILON, "CALCULATE_FROM", CALCULATE_FROM);
    }

    friend std::ostream &operator<<(std::ostream &os, const AugmentedSketchConfig &config) {
        ConfigPrinter<AugmentedSketchConfig>::print(os, config);
        return os;
    }
};

struct SpaceSavingConfig {
    int K;
    static void add_params_to_config_parser(SpaceSavingConfig &spacesaving_config, ConfigParser &parser) {
        // SpaceSaving configs prefix will be "spacesaving."
        parser.AddParameter(new IntParameter("spacesaving.k", "100", &spacesaving_config.K, false, "Number of heavy hitters to keep"));
    }

    auto to_tuple() const { return std::make_tuple("K", K); }
    friend std::ostream &operator<<(std::ostream &os, const SpaceSavingConfig &config) {
        ConfigPrinter<SpaceSavingConfig>::print(os, config);
        return os;
    }
};

struct WeightedFrequentConfig {
    unsigned int N;
    float EPSILON;
    string CALCULATE_FROM;

    static void add_params_to_config_parser(WeightedFrequentConfig &weightedfrequent_config, ConfigParser &parser) {
        // WeightedFrequent configs prefix will be "weightedfrequent."
        parser.AddParameter(new UnsignedInt32Parameter("weightedfrequent.N", "100", &weightedfrequent_config.N, false, "Number of heavy hitters to keep"));
        parser.AddParameter(new FloatParameter("weightedfrequent.epsilon", "0.01", &weightedfrequent_config.EPSILON, false, "Epsilon value for the weighted frequent"));
        parser.AddParameter(
            new StringParameter("weightedfrequent.calculate_from", "N", &weightedfrequent_config.CALCULATE_FROM, false, "Calculate the weighted frequent from N or EPSILON"));
    }

    auto to_tuple() const { return std::make_tuple("N", N, "EPSILON", EPSILON, "CALCULATE_FROM", CALCULATE_FROM); }
    friend std::ostream &operator<<(std::ostream &os, const WeightedFrequentConfig &config) {
        ConfigPrinter<WeightedFrequentConfig>::print(os, config);
        return os;
    }
};

struct OptimizedWeightedFrequentConfig : public WeightedFrequentConfig {};
