# No-Qt smoke: links FleetSimDomain + FleetSimCore only (no Qt / no rclcpp).
add_executable(fleet_domain_smoke main.cpp)

target_link_libraries(fleet_domain_smoke
    PRIVATE
        FleetSimDomain
        FleetSimCore
)

target_compile_features(fleet_domain_smoke PRIVATE cxx_std_17)
