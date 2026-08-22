set(CMAKE_AUTOMOC ON)

add_library(FleetSimApp STATIC
    SimController.cpp
    SimController.h
    ProjectManager.cpp
    ProjectManager.h
    MonitorBridge.cpp
    MonitorBridge.h
)

target_include_directories(FleetSimApp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FleetSimApp
    PUBLIC
        FleetSimDomain
        Qt6::Core
)

target_compile_features(FleetSimApp PUBLIC cxx_std_17)
