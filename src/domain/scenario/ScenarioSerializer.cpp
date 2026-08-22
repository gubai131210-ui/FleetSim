#include "ScenarioSerializer.h"

#include "domain/map/MapLoader.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <stdexcept>

namespace fleetsim::domain::scenario {

namespace {

core::Pose parsePose(const nlohmann::json& pose_json)
{
    core::Pose pose;
    pose.x = pose_json.at("x").get<double>();
    pose.y = pose_json.at("y").get<double>();
    pose.theta = pose_json.at("theta").get<double>();
    return pose;
}

}  // namespace

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
        vehicle.wheelbase_m = vehicle_json.value("wheelbase_m", 0.8);
        vehicle.max_steering_rad = vehicle_json.value("max_steering_rad", 0.6);
        vehicle.initial_pose = parsePose(vehicle_json.at("pose"));
        data.vehicles.push_back(std::move(vehicle));
    }

    if (json.contains("tasks")) {
        for (const auto& task_json : json.at("tasks")) {
            core::Task task;
            task.id = task_json.at("id").get<std::string>();
            task.pickup = parsePose(task_json.at("pickup"));
            task.dropoff = parsePose(task_json.at("dropoff"));
            task.priority = task_json.value("priority", 0);
            task.status = core::TaskStatus::Pending;
            data.tasks.push_back(std::move(task));
        }
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

    nlohmann::json tasks = nlohmann::json::array();
    for (const core::Task& task : scenario.tasks) {
        tasks.push_back({
            {"id", task.id},
            {"pickup",
             {
                 {"x", task.pickup.x},
                 {"y", task.pickup.y},
                 {"theta", task.pickup.theta},
             }},
            {"dropoff",
             {
                 {"x", task.dropoff.x},
                 {"y", task.dropoff.y},
                 {"theta", task.dropoff.theta},
             }},
            {"priority", task.priority},
        });
    }
    json["tasks"] = tasks;

    nlohmann::json vehicles = nlohmann::json::array();
    for (const VehicleConfig& vehicle : scenario.vehicles) {
        nlohmann::json vehicle_json = {
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
        };
        if (vehicle.model == "bicycle") {
            vehicle_json["wheelbase_m"] = vehicle.wheelbase_m;
            vehicle_json["max_steering_rad"] = vehicle.max_steering_rad;
        }
        vehicles.push_back(std::move(vehicle_json));
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
