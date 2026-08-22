add_library(FleetSimCore STATIC
    types/Pose.h
    types/ControlCommand.h
    types/VehicleId.h
    types/Waypoint.h
    types/Path.h
    types/GridCell.h
    types/Task.h
    types/TimeWindow.h
    types/VehicleState.h
    SimClock.cpp
    SimClock.h
    EventBus.cpp
    EventBus.h
)
