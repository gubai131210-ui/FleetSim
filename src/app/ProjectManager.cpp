#include "ProjectManager.h"

#include "domain/map/MapSerializer.h"
#include "domain/scenario/ScenarioSerializer.h"

#include <filesystem>

namespace fleetsim::app {

domain::map::MapDocument& ProjectManager::mapDocument()
{
    return map_document_;
}

const domain::map::MapDocument& ProjectManager::mapDocument() const
{
    return map_document_;
}

domain::scenario::ScenarioData& ProjectManager::scenarioData()
{
    return scenario_data_;
}

const domain::scenario::ScenarioData& ProjectManager::scenarioData() const
{
    return scenario_data_;
}

bool ProjectManager::load(const std::string& scenario_directory, double inflation_radius_m)
{
    const std::filesystem::path dir(scenario_directory);
    const auto map_path = dir / "map.json";
    const auto scenario_path = dir / "scenario.json";

    if (!std::filesystem::exists(map_path) || !std::filesystem::exists(scenario_path)) {
        loaded_ = false;
        return false;
    }

    map_document_ = domain::map::MapSerializer::fromFile(map_path.string());
    scenario_data_ = domain::scenario::ScenarioSerializer::fromFile(
        scenario_path.string(), scenario_directory, inflation_radius_m);
    project_directory_ = scenario_directory;
    loaded_ = true;
    return true;
}

bool ProjectManager::save(const std::string& scenario_directory) const
{
    if (!loaded_) {
        return false;
    }

    std::filesystem::create_directories(scenario_directory);
    const auto map_path = std::filesystem::path(scenario_directory) / "map.json";
    const auto scenario_path = std::filesystem::path(scenario_directory) / "scenario.json";

    domain::scenario::ScenarioData scenario_copy = scenario_data_;
    scenario_copy.scenario_directory = scenario_directory;

    return domain::map::MapSerializer::saveToFile(map_document_, map_path.string())
        && domain::scenario::ScenarioSerializer::saveToFile(scenario_copy, scenario_path.string());
}

domain::map::OccupancyGrid ProjectManager::buildOccupancyGrid(double inflation_radius_m) const
{
    return domain::map::MapSerializer::toOccupancyGrid(map_document_, inflation_radius_m);
}

void ProjectManager::setDefaultMapSize(double width_m, double height_m, double resolution_m)
{
    map_document_.width_m = width_m;
    map_document_.height_m = height_m;
    map_document_.grid_resolution_m = resolution_m;
}

void ProjectManager::newProject(const std::string& scenario_directory)
{
    project_directory_ = scenario_directory;
    loaded_ = true;

    map_document_ = domain::map::MapDocument{};
    map_document_.width_m = 20.0;
    map_document_.height_m = 15.0;
    map_document_.grid_resolution_m = 0.1;

    scenario_data_ = domain::scenario::ScenarioData{};
    scenario_data_.scenario_directory = scenario_directory;
    scenario_data_.simulation.dt_s = 0.05;
    scenario_data_.simulation.realtime = false;

    domain::scenario::VehicleConfig vehicle;
    vehicle.id = "agv_0";
    vehicle.model = "diff_drive";
    vehicle.svg_path = "assets/vehicles/agv_diff.svg";
    vehicle.length_m = 1.0;
    vehicle.initial_pose = {2.0, 2.0, 0.0};
    scenario_data_.vehicles = {vehicle};
}

}  // namespace fleetsim::app
