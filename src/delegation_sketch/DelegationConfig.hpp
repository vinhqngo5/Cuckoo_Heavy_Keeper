#pragma once

#include "utils/ConfigParser.hpp"
#include "utils/ConfigPrinter.hpp"
#include <getopt.h>
#include <string>

struct DelegationConfig {
    int FILTER_SIZE;
    double QUERY_RATE;

    static void add_params_to_config_parser(DelegationConfig &delegation_Config, ConfigParser &parser) {
        // Delegation configs prefix will be "delegation."
        parser.AddParameter(new IntParameter("delegation.filter_size", "16", &delegation_Config.FILTER_SIZE, false, "Filter size for the delegation sketch"));

        parser.AddParameter(new DoubleParameter("delegation.query_rate", "0", &delegation_Config.QUERY_RATE, false, "Query rate for the delegation sketch"));
    }

    auto to_tuple() const { return std::make_tuple("FILTER_SIZE", FILTER_SIZE, "QUERY_RATE", QUERY_RATE); }

    friend std::ostream &operator<<(std::ostream &os, const DelegationConfig &config) {
        ConfigPrinter<DelegationConfig>::print(os, config);
        return os;
    }
};

struct DelegationHeavyHitterConfig {
    int FILTER_SIZE;
    double QUERY_RATE;
    double HEAVY_QUERY_RATE;   // rate for querying the all heavy hitters
    static void add_params_to_config_parser(DelegationHeavyHitterConfig &delegation_heavyhitter_config, ConfigParser &parser) {
        // DelegationHeavyHitter configs prefix will be "delegationheavyhitter."
        parser.AddParameter(
            new IntParameter("delegationheavyhitter.filter_size", "16", &delegation_heavyhitter_config.FILTER_SIZE, false, "Filter size for the delegation heavy hitter sketch"));

        parser.AddParameter(
            new DoubleParameter("delegationheavyhitter.query_rate", "0", &delegation_heavyhitter_config.QUERY_RATE, false, "Query rate for the delegation heavy hitter sketch"));
        parser.AddParameter(new DoubleParameter("delegationheavyhitter.heavy_query_rate", "0.1", &delegation_heavyhitter_config.HEAVY_QUERY_RATE, false,
                                                "Query rate for the delegation heavy hitter sketch for querying all heavy hitters"));
    }

    // cast from DelegationHeavyHitterConfig to DelegationConfig
    operator DelegationConfig() const {
        DelegationConfig delegation_configs;
        delegation_configs.FILTER_SIZE = this->FILTER_SIZE;
        delegation_configs.QUERY_RATE = this->QUERY_RATE;
        return delegation_configs;
    }

    auto to_tuple() const { return std::make_tuple("FILTER_SIZE", FILTER_SIZE, "QUERY_RATE", QUERY_RATE, "HEAVY_QUERY_RATE", HEAVY_QUERY_RATE); }

    friend std::ostream &operator<<(std::ostream &os, const DelegationHeavyHitterConfig &config) {
        ConfigPrinter<DelegationHeavyHitterConfig>::print(os, config);
        return os;
    }
};
