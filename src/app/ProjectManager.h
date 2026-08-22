#pragma once

#include "domain/map/MapData.h"
#include "domain/scenario/ScenarioLoader.h"

#include <string>

namespace fleetsim::app {

class ProjectManager {
public:
    bool load(const std::string& scenario_directory, double inflation_radius_m = 0.55);
    bool save(const std::string& scenario_directory) const;

    bool hasProject() const { return loaded_; }
    const std::string& projectDirectory() const { return project_directory_; }

    map::MapDocument& mapDocument();
    const map::MapDocument& mapDocument() const;

    scenario::ScenarioData& scenarioData();
    const scenario::ScenarioData& scenarioData() const;

    map::OccupancyGrid buildOccupancyGrid(double inflation_radius_m = 0.55) const;

    void setDefaultMapSize(double width_m, double height_m, double resolution_m = 0.1);
    void newProject(const std::string& scenario_directory);

private:
    bool loaded_{false};
    std::string project_directory_;
    map::MapDocument map_document_;
    scenario::ScenarioData scenario_data_;
};

}  // namespace fleetsim::app
