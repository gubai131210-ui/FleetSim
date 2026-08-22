#include "CollisionModule.h"

namespace fleetsim::domain::collision {

void CollisionModule::tick(double /*dt*/)
{
    // Phase 3: detect and resolve multi-AGV conflicts.
}

int CollisionModule::conflictCount() const
{
    return conflicts_;
}

}  // namespace fleetsim::domain::collision
