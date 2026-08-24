#include "domain/map/LaneGraph.h"
#include "domain/map/OsmLaneletImporter.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

using fleetsim::domain::map::LaneGraph;
using fleetsim::domain::map::LaneMapData;
using fleetsim::domain::map::OsmImportError;
using fleetsim::domain::map::OsmLaneletImporter;

namespace {

std::string teachingOsmPath()
{
    const std::filesystem::path source_root =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_root / "assets" / "maps" / "teaching_lanelet_subset.osm").string();
}

}  // namespace

TEST(OsmLaneletImporterTest, BadXmlReturnsError)
{
    OsmImportError error;
    const auto result = OsmLaneletImporter::importFromXmlString("<not-valid-osm", &error);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(error.message.empty());
}

TEST(OsmLaneletImporterTest, ValidTeachingOsmProducesNonEmptyGraph)
{
    OsmImportError error;
    const auto lanes = OsmLaneletImporter::importFromFile(teachingOsmPath(), &error);
    ASSERT_TRUE(lanes.has_value()) << error.message;
    EXPECT_GE(lanes->nodes.size(), 2U);
    EXPECT_GE(lanes->edges.size(), 1U);
}

TEST(OsmLaneletImporterTest, ImportedGraphSupportsShortestPath)
{
    OsmImportError error;
    const auto lanes = OsmLaneletImporter::importFromFile(teachingOsmPath(), &error);
    ASSERT_TRUE(lanes.has_value()) << error.message;

    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(*lanes));
    EXPECT_FALSE(graph.empty());

    const std::string from_id = lanes->nodes.front().id;
    const std::string to_id = lanes->nodes.back().id;
    const auto path = graph.shortestPath(from_id, to_id);
    ASSERT_TRUE(path.has_value());
    EXPECT_GE(path->size(), 2U);
}

TEST(OsmLaneletImporterTest, MissingFileFails)
{
    OsmImportError error;
    const auto result = OsmLaneletImporter::importFromFile("nonexistent_phase10.osm", &error);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(error.message.empty());
}
