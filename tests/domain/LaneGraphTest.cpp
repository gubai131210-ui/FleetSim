#include "domain/map/LaneGraph.h"

#include <gtest/gtest.h>

using fleetsim::domain::map::LaneEdge;
using fleetsim::domain::map::LaneGraph;
using fleetsim::domain::map::LaneMapData;
using fleetsim::domain::map::LaneNode;

namespace {

LaneMapData makeForkGraph()
{
    LaneMapData data;
    data.nodes = {
        {"n0", 0.0, 0.0},
        {"n1", 4.0, 0.0},
        {"n2", 4.0, 4.0},
        {"n3", 8.0, 0.0},
    };
    data.edges = {
        {"n0", "n1", false},
        {"n1", "n2", false},
        {"n1", "n3", false},
        {"n2", "n3", false},
    };
    return data;
}

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

TEST(LaneGraphTest, LoadFromMapSetsNodeAndEdgeCounts)
{
    LaneGraph graph;
    const LaneMapData data = makeForkGraph();
    ASSERT_TRUE(graph.loadFromMap(data));
    EXPECT_EQ(graph.nodeCount(), 4U);
    EXPECT_EQ(graph.edgeCount(), 4U);
    EXPECT_FALSE(graph.empty());
}

TEST(LaneGraphTest, ShortestPathLinearChannel)
{
    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(makeLinearGraph()));

    const auto path = graph.shortestPath("a", "c");
    ASSERT_TRUE(path.has_value());
    ASSERT_EQ(path->size(), 3U);
    EXPECT_EQ((*path)[0], "a");
    EXPECT_EQ((*path)[1], "b");
    EXPECT_EQ((*path)[2], "c");
}

TEST(LaneGraphTest, ShortestPathForkChoosesShorterRoute)
{
    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(makeForkGraph()));

    const auto path = graph.shortestPath("n0", "n3");
    ASSERT_TRUE(path.has_value());
    ASSERT_GE(path->size(), 2U);
    EXPECT_EQ(path->front(), "n0");
    EXPECT_EQ(path->back(), "n3");
    // Direct n0→n1→n3 (8 m) beats n0→n1→n2→n3 (~12 m).
    ASSERT_EQ(path->size(), 3U);
    EXPECT_EQ((*path)[1], "n1");
}

TEST(LaneGraphTest, ShortestPathUnreachableReturnsNullopt)
{
    LaneGraph graph;
    LaneMapData data;
    data.nodes = {{"x", 0.0, 0.0}, {"y", 1.0, 0.0}};
    data.edges = {{"x", "y", false}};
    ASSERT_TRUE(graph.loadFromMap(data));

    EXPECT_FALSE(graph.shortestPath("y", "x").has_value());
}

TEST(LaneGraphTest, NearestNodeIdPicksClosest)
{
    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(makeLinearGraph()));

    EXPECT_EQ(graph.nearestNodeId(0.1, 0.1), "a");
    EXPECT_EQ(graph.nearestNodeId(5.9, 0.0), "c");
}

TEST(LaneGraphTest, CenterlinePathHasWaypointsForEachNode)
{
    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(makeLinearGraph()));

    const auto path = graph.centerlinePath({"a", "b", "c"});
    EXPECT_GE(path.size(), 2U);
    EXPECT_DOUBLE_EQ(path.waypoints().front().x, 0.0);
    EXPECT_DOUBLE_EQ(path.waypoints().back().x, 6.0);
}
