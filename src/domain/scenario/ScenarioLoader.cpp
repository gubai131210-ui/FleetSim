#include "ScenarioLoader.h"

#include "ScenarioSerializer.h"

#include <filesystem>

namespace fleetsim::domain::scenario {

ScenarioData ScenarioLoader::loadFromDirectory(const std::string& scenario_directory,
                                               double inflation_radius_m)
{
    const std::filesystem::path dir(scenario_directory);
    const std::filesystem::path scenario_path = dir / "scenario.json";
    return ScenarioSerializer::fromFile(scenario_path.string(), scenario_directory, inflation_radius_m);
}

}  // namespace fleetsim::domain::scenario
