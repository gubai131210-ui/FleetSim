enable_testing()

add_executable(FleetSimTests
    core/SimClockTest.cpp
    core/EventBusTest.cpp
    domain/SimEngineTest.cpp
    domain/SchedulingModuleTest.cpp
    domain/CollisionModuleTest.cpp
    domain/OccupancyGridTest.cpp
    domain/MapLoaderTest.cpp
    domain/AStarPlannerTest.cpp
    domain/PurePursuitTrackerTest.cpp
    domain/VehicleTest.cpp
    domain/ScenarioLoaderTest.cpp
    integration/DemoScenarioTest.cpp
)

target_link_libraries(FleetSimTests
    PRIVATE
        FleetSimDomain
        FleetSimCore
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(FleetSimTests)
