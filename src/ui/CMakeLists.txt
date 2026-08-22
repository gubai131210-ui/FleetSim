qt_add_library(FleetSimUI STATIC
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    map/MapScene.cpp
    map/MapScene.h
    map/MapView.cpp
    map/MapView.h
    graphics/VehicleGraphicsItem.cpp
    graphics/VehicleGraphicsItem.h
)

target_include_directories(FleetSimUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FleetSimUI
    PUBLIC
        FleetSimApp
        Qt6::Core
        Qt6::Widgets
        Qt6::Svg
)

target_compile_features(FleetSimUI PUBLIC cxx_std_17)
