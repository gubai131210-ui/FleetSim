add_library(FleetSimCore STATIC
    types/Pose.h
    types/ControlCommand.h
    types/VehicleId.h
    types/Waypoint.h
    types/Path.h
    types/GridCell.h
    SimClock.cpp
    SimClock.h
    EventBus.cpp
    EventBus.h
)

target_include_directories(FleetSimCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FleetSimCore
    PUBLIC
        Eigen3::Eigen
)

target_compile_features(FleetSimCore PUBLIC cxx_std_17)

fleetsim_enable_coverage(FleetSimCore)
