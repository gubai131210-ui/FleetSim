set(CMAKE_AUTOMOC ON)

add_library(FleetSimApp STATIC
    SimController.cpp
    SimController.h
    ProjectManager.cpp
    ProjectManager.h
    MonitorBridge.cpp
    MonitorBridge.h
    FleetUiCoordinator.cpp
    FleetUiCoordinator.h
)

target_include_directories(FleetSimApp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../ui
)

target_link_libraries(FleetSimApp
    PUBLIC
        FleetSimDomain
        Qt6::Core
        Qt6::Widgets
        Qt6::Svg
)

target_compile_features(FleetSimApp PUBLIC cxx_std_17)
