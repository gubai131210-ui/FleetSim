#pragma once

#include "ScenarioLoader.h"

#include <nlohmann/json_fwd.hpp>

#include <string>

namespace fleetsim::domain::scenario {

class ScenarioSerializer {
public:
    static ScenarioData fromJson(const nlohmann::json& json, const std::string& scenario_directory);
    static ScenarioData fromFile(const std::string& scenario_json_path,
                                 const std::string& scenario_directory,
                                 double inflation_radius_m = 0.55);

    static nlohmann::json toJson(const ScenarioData& scenario);
    static bool saveToFile(const ScenarioData& scenario, const std::string& scenario_json_path);
};

}  // namespace fleetsim::domain::scenario
