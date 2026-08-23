#include "domain/planning/LaneRouter.h"

#include "domain/map/LaneGraph.h"

#include <gtest/gtest.h>

using fleetsim::domain::map::LaneEdge;
using fleetsim::domain::map::LaneGraph;
using fleetsim::domain::map::LaneMapData;
using fleetsim::domain::map::LaneNode;
using fleetsim::domain::planning::LaneRouter;

namespace {

LaneMapData makeLinearGraph()
{
    LaneMapData data;
    data.nodes = {
        {"a", 0.0, 0.0},
        {"b", 3.0, 0.0},
        {"c", 6.0, 0.0},
    };
    data.edges = {
        {"a", "b", false},
        {"b", "c", false},
    };
    return data;
}

}  // namespace

TEST(LaneRouterTest, RouteEmptyGraphFails)
{
    LaneGraph graph;
    LaneRouter router(graph);
    EXPECT_FALSE(router.route("a", "b").has_value());
}

TEST(LaneRouterTest, RouteLinearChannelProducesPath)
{
    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(makeLinearGraph()));

    LaneRouter router(graph);
    const auto path = router.route("a", "c");
    ASSERT_TRUE(path.has_value());
    EXPECT_GE(path->size(), 2U);
    EXPECT_GT(path->length(), 0.0);
}

TEST(LaneRouterTest, RouteUnreachableReturnsNullopt)
{
    LaneGraph graph;
    LaneMapData data;
    data.nodes = {{"x", 0.0, 0.0}, {"y", 1.0, 0.0}};
    data.edges = {{"x", "y", false}};
    ASSERT_TRUE(graph.loadFromMap(data));

    LaneRouter router(graph);
    EXPECT_FALSE(router.route("y", "x").has_value());
}
