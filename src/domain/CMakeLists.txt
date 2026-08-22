add_library(FleetSimDomain STATIC
    SimEngine.cpp
    SimEngine.h
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
