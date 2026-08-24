#include "domain/collision/CbsLiteCoordinator.h"

namespace fleetsim::domain::collision {

CbsLiteResult CbsLiteCoordinator::resolve(const vehicle::FleetManager& /*fleet*/,
                                          const map::OccupancyGrid& /*grid*/,
                                          const CbsLiteConfig& /*config*/)
{
    CbsLiteResult result;
    result.success = false;
    result.constraints_added = 0;
    result.nodes_expanded = 0;
    result.message = "CbsLiteCoordinator not implemented (Phase 10 Session 5 pending)";
    return result;
}

}  // namespace fleetsim::domain::collision
