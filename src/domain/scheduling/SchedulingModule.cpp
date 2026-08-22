#include "SchedulingModule.h"

namespace fleetsim::domain::scheduling {

void SchedulingModule::tick(double /*dt*/)
{
    // Phase 3: assign tasks to idle AGVs.
}

int SchedulingModule::pendingTaskCount() const
{
    return pending_tasks_;
}

}  // namespace fleetsim::domain::scheduling
