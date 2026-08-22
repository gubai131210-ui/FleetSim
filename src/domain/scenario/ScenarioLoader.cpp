#include "ScenarioLoader.h"

#include "domain/map/MapLoader.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <stdexcept>

namespace fleetsim::domain::scenario {

ScenarioData ScenarioLoader::loadFromDirectory(const std::string& scenario_directory,
                                               double inflation_radius_m)
{
    const std::filesystem::path dir(scenario_directory);
    const std::filesystem::path map_path = dir / "map.json";
    const std::filesystem::path scenario_path = dir / "scenario.json";

    std::ifstream scenario_input(scenario_path);
    if (!scenario_input.is_open()) {
        throw std::runtime_error("Failed to open scenario file: " + scenario_path.string());
    }

    nlohmann::json scenario_json;
    scenario_input >> scenario_json;

    ScenarioData data;
    data.scenario_directory = scenario_directory;
    data.map = map::MapLoader::loadFromFile(map_path.string(), inflation_radius_m);

    const auto& simulation = scenario_json.at("simulation");
    data.simulation.dt_s = simulation.at("dt_s").get<double>();
    data.simulation.realtime = simulation.value("realtime", false);

    for (const auto& vehicle_json : scenario_json.at("vehicles")) {
        VehicleConfig vehicle;
        vehicle.id = vehicle_json.at("id").get<std::string>();
        vehicle.model = vehicle_json.at("model").get<std::string>();
        vehicle.svg_path = vehicle_json.at("svg").get<std::string>();
        vehicle.length_m = vehicle_json.value("length_m", 1.0);

        const auto& pose = vehicle_json.at("pose");
        vehicle.initial_pose.x = pose.at("x").get<double>();
        vehicle.initial_pose.y = pose.at("y").get<double>();
        vehicle.initial_pose.theta = pose.at("theta").get<double>();

        data.vehicles.push_back(std::move(vehicle));
    }

    return data;
}

}  // namespace fleetsim::domain::scenario
