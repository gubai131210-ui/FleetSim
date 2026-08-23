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
    planning/HybridAStarPlanner.cpp
    planning/HybridAStarPlanner.h
    planning/DubinsPath.cpp
    planning/DubinsPath.h
    planning/DouglasPeuckerSmoother.cpp
    planning/DouglasPeuckerSmoother.h
    planning/StGraphSpeedPlanner.cpp
    planning/StGraphSpeedPlanner.h
    planning/IPathPlanner.h
    planning/IPathSmoother.h
    control/PurePursuitTracker.cpp
    control/PurePursuitTracker.h
    control/StanleyTracker.cpp
    control/StanleyTracker.h
    control/MpcLateralTracker.cpp
    control/MpcLateralTracker.h
    control/SteeringAdapter.cpp
    control/SteeringAdapter.h
    control/IPathTracker.h
    vehicle/IVehicleModel.h
    vehicle/DiffDriveModel.cpp
    vehicle/DiffDriveModel.h
    vehicle/BicycleModel.cpp
    vehicle/BicycleModel.h
    vehicle/VehicleModelFactory.cpp
    vehicle/VehicleModelFactory.h
    vehicle/Vehicle.cpp
    vehicle/Vehicle.h
    vehicle/FleetManager.cpp
    vehicle/FleetManager.h
    scenario/ScenarioLoader.cpp
    scenario/ScenarioLoader.h
    scenario/ScenarioSerializer.cpp
    scenario/ScenarioSerializer.h
    scheduling/SchedulingModule.cpp
    scheduling/SchedulingModule.h
    scheduling/TaskQueue.cpp
    scheduling/TaskQueue.h
    scheduling/GreedyAssigner.cpp
    scheduling/GreedyAssigner.h
    scheduling/HungarianAssigner.cpp
    scheduling/HungarianAssigner.h
    scheduling/ITaskAssigner.h
    collision/CollisionModule.cpp
    collision/CollisionModule.h
    collision/PathReservationTable.cpp
    collision/PathReservationTable.h
    collision/TimeWindowCollisionAvoidance.cpp
    collision/TimeWindowCollisionAvoidance.h
    collision/PriorityPathCoordinator.cpp
    collision/PriorityPathCoordinator.h
    collision/ICollisionAvoidance.h
)

include(GNUInstallDirs)

target_include_directories(FleetSimDomain
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(FleetSimDomain
    PUBLIC
        FleetSimCore
        # FetchContent nlohmann is not in our export set; expose only at build time.
        # Installed package re-attaches via find_dependency + INTERFACE in Config.cmake.in.
        $<BUILD_INTERFACE:nlohmann_json::nlohmann_json>
)

target_compile_features(FleetSimDomain PUBLIC cxx_std_17)

fleetsim_enable_coverage(FleetSimDomain)

install(TARGETS FleetSimDomain
    EXPORT FleetSimDomainTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/domain
    FILES_MATCHING
        PATTERN "*.h"
)

install(EXPORT FleetSimDomainTargets
    FILE FleetSimDomainTargets.cmake
    NAMESPACE FleetSim::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FleetSimDomain
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/FleetSimDomainConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FleetSimDomainConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FleetSimDomain
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/FleetSimDomainConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/FleetSimDomainConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FleetSimDomainConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FleetSimDomain
)
