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
