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
    graphics/PathGraphicsItem.cpp
    graphics/PathGraphicsItem.h
    graphics/ObstacleOverlayItem.cpp
    graphics/ObstacleOverlayItem.h
    graphics/ObstacleGraphicsItem.cpp
    graphics/ObstacleGraphicsItem.h
    panels/ControlPanel.cpp
    panels/ControlPanel.h
    panels/MapEditorPanel.cpp
    panels/MapEditorPanel.h
    panels/MonitorPanel.cpp
    panels/MonitorPanel.h
    panels/TaskPanel.cpp
    panels/TaskPanel.h
    panels/VehicleInfoPanel.cpp
    panels/VehicleInfoPanel.h
    dialogs/ProjectDialog.cpp
    dialogs/ProjectDialog.h
)

target_include_directories(FleetSimUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FleetSimUI
    PUBLIC
        FleetSimApp
        QCustomPlot
        Qt6::Core
        Qt6::Widgets
        Qt6::Svg
)

target_compile_features(FleetSimUI PUBLIC cxx_std_17)
