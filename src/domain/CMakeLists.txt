add_library(FleetSimDomain STATIC
    SimEngine.cpp
    SimEngine.h
    map/OccupancyGrid.cpp
    map/OccupancyGrid.h
    map/MapLoader.cpp
    map/MapLoader.h
    map/MapData.h
    map/MapSerializer.cpp
    map/MapSerializer.h
    map/LaneGraph.cpp
    map/LaneGraph.h
    planning/AStarPlanner.cpp
    planning/AStarPlanner.h
    planning/DouglasPeuckerSmoother.cpp
    planning/DouglasPeuckerSmoother.h
    planning/IPathPlanner.h
    planning/IPathSmoother.h
    control/PurePursuitTracker.cpp
    control/PurePursuitTracker.h
    control/IPathTracker.h
    vehicle/DiffDriveModel.cpp
    vehicle/DiffDriveModel.h
    vehicle/Vehicle.cpp
    vehicle/Vehicle.h
    scenario/ScenarioLoader.cpp
    scenario/ScenarioLoader.h
    scenario/ScenarioSerializer.cpp
    scenario/ScenarioSerializer.h
    scheduling/SchedulingModule.cpp
    scheduling/SchedulingModule.h
    collision/CollisionModule.cpp
    collision/CollisionModule.h
)

target_include_directories(FleetSimDomain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FleetSimDomain
    PUBLIC
        FleetSimCore
        nlohmann_json::nlohmann_json
)

target_compile_features(FleetSimDomain PUBLIC cxx_std_17)

fleetsim_enable_coverage(FleetSimDomain)
