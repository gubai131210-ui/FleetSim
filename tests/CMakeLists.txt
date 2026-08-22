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
    domain/PurePursuitTrackerTest.cpp
    domain/SteeringAdapterTest.cpp
    domain/VehicleTest.cpp
    domain/BicycleModelTest.cpp
    domain/BicycleScenarioTest.cpp
    domain/ScenarioLoaderTest.cpp
    domain/ScenarioSerializerTest.cpp
    domain/ProjectManagerTest.cpp
    domain/TaskQueueTest.cpp
    domain/GreedyAssignerTest.cpp
    domain/PathReservationTableTest.cpp
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

include(GoogleTest)
gtest_discover_tests(FleetSimTests)
