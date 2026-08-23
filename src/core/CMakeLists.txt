add_library(FleetSimCore STATIC
    types/Pose.h
    types/ControlCommand.h
    types/VehicleId.h
    types/Waypoint.h
    types/Path.h
    types/SpeedProfile.h
    types/GridCell.h
    types/Task.h
    types/TimeWindow.h
    types/VehicleState.h
    SimClock.cpp
    SimClock.h
    EventBus.cpp
    EventBus.h
)

include(GNUInstallDirs)

target_include_directories(FleetSimCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(FleetSimCore
    PUBLIC
        Eigen3::Eigen
)

target_compile_features(FleetSimCore PUBLIC cxx_std_17)

fleetsim_enable_coverage(FleetSimCore)

install(TARGETS FleetSimCore
    EXPORT FleetSimDomainTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/core
    FILES_MATCHING
        PATTERN "*.h"
)
