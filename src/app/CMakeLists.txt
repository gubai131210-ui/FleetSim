add_library(FleetSimApp STATIC
    SimController.cpp
    SimController.h
)

target_include_directories(FleetSimApp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FleetSimApp
    PUBLIC
        FleetSimDomain
)

target_compile_features(FleetSimApp PUBLIC cxx_std_17)
