#include "ScenarioSerializer.h"

#include "domain/map/MapLoader.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <stdexcept>

namespace fleetsim::domain::scenario {

ScenarioData ScenarioSerializer::fromJson(const nlohmann::json& json,
                                            const std::string& scenario_directory)
{
    ScenarioData data;
    data.scenario_directory = scenario_directory;

    const auto& simulation = json.at("simulation");
    data.simulation.dt_s = simulation.at("dt_s").get<double>();
    data.simulation.realtime = simulation.value("realtime", false);

    for (const auto& vehicle_json : json.at("vehicles")) {
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

ScenarioData ScenarioSerializer::fromFile(const std::string& scenario_json_path,
                                          const std::string& scenario_directory,
                                          double inflation_radius_m)
{
    std::ifstream input(scenario_json_path);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open scenario file: " + scenario_json_path);
    }

    nlohmann::json json;
    input >> json;

    ScenarioData data = fromJson(json, scenario_directory);

    const std::filesystem::path map_path =
        std::filesystem::path(scenario_directory) / "map.json";
    data.map = map::MapLoader::loadFromFile(map_path.string(), inflation_radius_m);
    return data;
}

nlohmann::json ScenarioSerializer::toJson(const ScenarioData& scenario)
{
    nlohmann::json json;
    json["version"] = 1;
    json["simulation"] = {
        {"dt_s", scenario.simulation.dt_s},
        {"realtime", scenario.simulation.realtime},
    };
    json["tasks"] = nlohmann::json::array();

    nlohmann::json vehicles = nlohmann::json::array();
    for (const VehicleConfig& vehicle : scenario.vehicles) {
        vehicles.push_back({
            {"id", vehicle.id},
            {"model", vehicle.model},
            {"svg", vehicle.svg_path},
            {"length_m", vehicle.length_m},
            {"pose",
             {
                 {"x", vehicle.initial_pose.x},
                 {"y", vehicle.initial_pose.y},
                 {"theta", vehicle.initial_pose.theta},
             }},
        });
    }
    json["vehicles"] = vehicles;
    return json;
}

bool ScenarioSerializer::saveToFile(const ScenarioData& scenario, const std::string& scenario_json_path)
{
    std::ofstream output(scenario_json_path);
    if (!output.is_open()) {
        return false;
    }
    output << toJson(scenario).dump(2);
    return output.good();
}

}  // namespace fleetsim::domain::scenario
