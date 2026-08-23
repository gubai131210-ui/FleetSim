enable_testing()

add_executable(FleetSimTests
    core/SimClockTest.cpp
    core/EventBusTest.cpp
    core/TimeWindowTest.cpp
    domain/SimEngineTest.cpp
    domain/SchedulingModuleTest.cpp
    domain/CollisionModuleTest.cpp
    domain/OccupancyGridTest.cpp
    domain/MapLoaderTest.cpp
    domain/MapSerializerTest.cpp
    domain/AStarPlannerTest.cpp
    domain/HybridAStarPlannerTest.cpp
    domain/DubinsPathTest.cpp
    domain/PurePursuitTrackerTest.cpp
    domain/StanleyTrackerTest.cpp
    domain/MpcLateralTrackerTest.cpp
    domain/DenseQpSolverTest.cpp
    domain/MpcVsStanleyCompareTest.cpp
    domain/StGraphSpeedPlannerTest.cpp
    domain/StGraphSimEngineWiringTest.cpp
    domain/SteeringAdapterTest.cpp
    domain/VehicleTest.cpp
    domain/BicycleModelTest.cpp
    domain/BicycleScenarioTest.cpp
    domain/PlannerSwitchIntegrationTest.cpp
    domain/ScenarioLoaderTest.cpp
    domain/ScenarioSerializerTest.cpp
    domain/ProjectManagerTest.cpp
    domain/TaskQueueTest.cpp
    domain/GreedyAssignerTest.cpp
    domain/HungarianAssignerTest.cpp
    domain/PathReservationTableTest.cpp
    domain/PriorityCoordinatorTest.cpp
    domain/FleetManagerTest.cpp
    integration/DemoScenarioTest.cpp
    integration/MultiAgvScenarioTest.cpp
)

target_link_libraries(FleetSimTests
    PRIVATE
        FleetSimApp
        FleetSimDomain
        FleetSimCore
        GTest::gtest_main
)

fleetsim_enable_coverage(FleetSimTests)

# Scenario/integration tests resolve assets relative to cwd or near the binary.
add_custom_command(TARGET FleetSimTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:FleetSimTests>/assets
    COMMENT "Copying assets next to FleetSimTests"
)

include(GoogleTest)
gtest_discover_tests(FleetSimTests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
