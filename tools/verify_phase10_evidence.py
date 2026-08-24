#!/usr/bin/env python3
"""Phase 10 static evidence audit (Session 7 full ≥60 checks)."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
fails: list[str] = []
passes: list[str] = []


def ok(msg: str) -> None:
    passes.append(msg)
    print(f"PASS  {msg}")


def bad(msg: str) -> None:
    fails.append(msg)
    print(f"FAIL  {msg}")


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.exists():
        bad(f"missing file: {rel}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def must_exist(rel: str, label: str) -> None:
    if (ROOT / rel).exists():
        ok(label)
    else:
        bad(label)


def must_contain(rel: str, pattern: str, label: str) -> None:
    text = read(rel)
    if not text:
        return
    if re.search(pattern, text, re.MULTILINE | re.DOTALL):
        ok(label)
    else:
        bad(f"{label} — not in {rel}")


def must_not_contain(rel: str, pattern: str, label: str) -> None:
    text = read(rel)
    if not text:
        return
    if re.search(pattern, text, re.MULTILINE | re.IGNORECASE):
        bad(f"{label} — forbidden pattern in {rel}")
    else:
        ok(label)


def line_count(rel: str) -> int:
    path = ROOT / rel
    if not path.exists():
        return 0
    return len(path.read_text(encoding="utf-8", errors="replace").splitlines())


def must_under_lines(rel: str, max_lines: int, label: str) -> None:
    count = line_count(rel)
    if count == 0:
        bad(f"{label} — missing {rel}")
    elif count <= max_lines:
        ok(f"{label} ({count} lines)")
    else:
        bad(f"{label} — {rel} has {count} lines (> {max_lines})")


def main() -> int:
    print(f"ROOT={ROOT}")
    print("--- Phase 10 evidence ---")

    print("--- A ADR ---")
    must_exist("docs/decisions/022-osm-lanelet-import.md", "A: ADR-022")
    must_exist("docs/decisions/023-bt-xml-motion-recovery.md", "A: ADR-023")
    must_exist("docs/decisions/024-cbs-lite.md", "A: ADR-024")
    must_exist("docs/decisions/025-multi-bt-ui-ia.md", "A: ADR-025")
    must_contain("docs/decisions/022-osm-lanelet-import.md", r"OsmLaneletImporter", "A: ADR-022 references importer")
    must_contain("docs/decisions/023-bt-xml-motion-recovery.md", r"BtXmlLoader|Spin|BackUp", "A: ADR-023 BT XML/recovery")
    must_contain("docs/decisions/024-cbs-lite.md", r"CbsLiteCoordinator|cbs_lite", "A: ADR-024 CBS-lite")
    must_contain("docs/decisions/025-multi-bt-ui-ia.md", r"MultiAgentBehaviorPanel|BehaviorXmlPage", "A: ADR-025 UI IA")

    print("--- B Osm / Lanelet ---")
    must_exist("src/domain/map/OsmLaneletImporter.h", "B: OsmLaneletImporter.h")
    must_exist("src/domain/map/OsmLaneletImporter.cpp", "B: OsmLaneletImporter.cpp")
    must_exist("tests/domain/OsmLaneletImporterTest.cpp", "B: OsmLaneletImporterTest")
    must_exist("assets/maps/teaching_lanelet_subset.osm", "B: teaching OSM asset")
    must_contain("src/domain/map/OsmLaneletImporter.cpp", r"buildCenterlineFromBounds|centerline", "B: centerline generation")
    must_contain("src/domain/map/OsmLaneletImporter.cpp", r"left_exit_osm|shared.*node", "B: node-id successor logic")
    must_contain("tests/domain/OsmLaneletImporterTest.cpp", r"ImportedGraphSupportsShortestPath", "B: shortestPath test")

    print("--- C BT XML / control ---")
    must_exist("src/domain/behavior/BtXmlLoader.h", "C: BtXmlLoader.h")
    must_exist("src/domain/behavior/BtXmlLoader.cpp", "C: BtXmlLoader.cpp")
    must_exist("tests/domain/BtXmlLoaderTest.cpp", "C: BtXmlLoaderTest")
    must_exist("assets/behavior_trees/navigate_spin_backup_recovery.xml", "C: Nav2-style BT XML")
    must_contain("src/domain/behavior/BtControlNodes.h", r"BtRoundRobinNode", "C: RoundRobin node")
    must_contain("src/domain/behavior/BtControlNodes.h", r"BtReactiveFallbackNode", "C: ReactiveFallback node")
    must_contain("assets/behavior_trees/navigate_spin_backup_recovery.xml", r"RoundRobin|Spin|BackUp", "C: XML asset recovery nodes")
    must_contain("tests/domain/BtXmlLoaderTest.cpp", r"LoadsNavigateSpinBackupAsset|RoundRobin", "C: BtXmlLoader asset test")

    print("--- D Motion recovery ---")
    must_exist("src/domain/behavior/BtMotionRecoveryNodes.h", "D: BtMotionRecoveryNodes.h")
    must_exist("src/domain/behavior/BtMotionRecoveryNodes.cpp", "D: BtMotionRecoveryNodes.cpp")
    must_exist("tests/domain/BtMotionRecoveryTest.cpp", "D: BtMotionRecoveryTest")
    must_contain("src/domain/behavior/BtMotionRecoveryNodes.cpp", r"Spin|yaw", "D: Spin recovery impl")
    must_contain("src/domain/behavior/BtMotionRecoveryNodes.cpp", r"BackUp|backup", "D: BackUp recovery impl")
    must_contain("src/domain/behavior/BtMotionRecoveryNodes.cpp", r"ClearInflation|clearInflation", "D: ClearInflation impl")
    must_contain("tests/domain/BtMotionRecoveryTest.cpp", r"Spin", "D: Spin test")
    must_contain("tests/domain/BtMotionRecoveryTest.cpp", r"BackUp|Backup", "D: BackUp test")
    must_contain("tests/domain/BtMotionRecoveryTest.cpp", r"ClearInflation|Clear", "D: ClearInflation test")

    print("--- E Multi BT ---")
    must_exist("src/domain/behavior/MultiBtNavigator.h", "E: MultiBtNavigator.h")
    must_exist("src/domain/behavior/MultiBtNavigator.cpp", "E: MultiBtNavigator.cpp")
    must_exist("tests/integration/MultiBtNavigationTest.cpp", "E: MultiBtNavigationTest")
    must_contain("src/domain/SimEngine.cpp", r"multiBtNavigator|multi_bt", "E: SimEngine multi BT tick")
    must_contain("tests/integration/MultiBtNavigationTest.cpp", r"SeparateBlackboardsDoNotOverwriteAgentId", "E: blackboard isolation test")

    print("--- F CBS-lite ---")
    must_exist("src/domain/collision/CbsLiteCoordinator.h", "F: CbsLiteCoordinator.h")
    must_exist("src/domain/collision/CbsLiteCoordinator.cpp", "F: CbsLiteCoordinator.cpp")
    must_exist("tests/domain/CbsLiteCoordinatorTest.cpp", "F: CbsLiteCoordinatorTest")
    must_contain("src/domain/SimEngine.cpp", r"replanFleetWithCbsLiteCoordination|cbs_lite", "F: SimEngine CBS wiring")
    must_contain("tests/domain/CbsLiteCoordinatorTest.cpp", r"HeadOnConflictProducesConstraints", "F: CBS conflict test")
    must_contain("tests/domain/CbsLiteCoordinatorTest.cpp", r"ZeroDepthFailsImmediately", "F: CBS depth limit test")

    print("--- G UI Phase 10 four-piece ---")
    must_exist("src/ui/panels/OsmImportPanel.h", "G: OsmImportPanel")
    must_exist("src/ui/panels/MultiAgentBehaviorPanel.h", "G: MultiAgentBehaviorPanel")
    must_exist("src/ui/dialogs/pages/MapImportPage.h", "G: MapImportPage")
    must_exist("src/ui/dialogs/pages/BehaviorXmlPage.h", "G: BehaviorXmlPage")
    must_contain("src/ui/dialogs/AlgorithmWorkbenchDialog.cpp", r"Map Import", "G: Workbench Tab 7")
    must_contain("src/ui/dialogs/AlgorithmWorkbenchDialog.cpp", r"Behavior XML", "G: Workbench Tab 8")
    must_contain("src/ui/dialogs/pages/CoordinationPage.cpp", r"cbs_lite", "G: Coordination cbs_lite")
    must_contain("src/ui/MainWindow.cpp", r"OsmImportPanel", "G: MainWindow OsmImportPanel")
    must_contain("src/ui/MainWindow.cpp", r"MultiAgentBehaviorPanel", "G: MainWindow MultiAgentBehaviorPanel")
    must_contain("src/ui/CMakeLists.txt", r"OsmImportPanel", "G: CMake OsmImportPanel")
    must_contain("src/ui/CMakeLists.txt", r"MapImportPage", "G: CMake MapImportPage")

    print("--- H UI negative audit (ADR-025) ---")
    must_not_contain(
        "src/ui/panels/MapEditorPanel.cpp", r"osm|OsmImport|\.osm", "H: MapEditorPanel no OSM pollution"
    )
    must_not_contain(
        "src/ui/panels/LaneEditorPanel.cpp", r"osm|OsmImport|\.osm", "H: LaneEditorPanel no OSM pollution"
    )
    must_not_contain(
        "src/ui/dialogs/pages/BehaviorPage.cpp",
        r"bt_format|RoundRobin|spin_rad|backup_dist",
        "H: BehaviorPage no XML/recovery pollution",
    )
    must_not_contain(
        "src/ui/dialogs/pages/RoutingPage.cpp",
        r"cbs_lite|osm_path",
        "H: RoutingPage no CBS/OSM pollution",
    )
    must_not_contain(
        "src/ui/dialogs/pages/CoordinationPage.cpp",
        r"behavior_tree|osm",
        "H: CoordinationPage no BT/OSM pollution",
    )

    print("--- I UI line limits ---")
    must_under_lines("src/ui/panels/OsmImportPanel.cpp", 300, "I: OsmImportPanel.cpp ≤300")
    must_under_lines("src/ui/panels/MultiAgentBehaviorPanel.cpp", 300, "I: MultiAgentBehaviorPanel.cpp ≤300")
    must_under_lines("src/ui/dialogs/pages/MapImportPage.cpp", 300, "I: MapImportPage.cpp ≤300")
    must_under_lines("src/ui/dialogs/pages/BehaviorXmlPage.cpp", 300, "I: BehaviorXmlPage.cpp ≤300")

    print("--- J Scenario / App wiring ---")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"map_source", "J: scenario map_source")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"bt_format", "J: scenario bt_format")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"cbs_max_depth", "J: scenario cbs_max_depth")
    must_contain("src/domain/scenario/ScenarioSerializer.cpp", r"osm_path", "J: serializer osm_path")
    must_contain("src/app/SimController.cpp", r"applyOsmLaneMap|OsmLaneletImporter", "J: SimController OSM load")
    must_contain("src/app/SimController.cpp", r"setCbsLiteConfig", "J: SimController CBS config")
    must_contain("src/app/MonitorBridge.cpp", r"multiAgentBehaviorUpdated|MultiAgent", "J: MonitorBridge multi-agent BT")

    print("--- K Demo scenarios ---")
    must_exist("assets/scenarios/osm_lanelet_demo/scenario.json", "K: osm_lanelet_demo scenario")
    must_exist("assets/scenarios/osm_lanelet_demo/map.json", "K: osm_lanelet_demo map")
    must_contain("assets/scenarios/osm_lanelet_demo/scenario.json", r'"map_source"\s*:\s*"osm"', "K: osm demo map_source")
    must_exist("assets/scenarios/cbs_lite_demo/scenario.json", "K: cbs_lite_demo scenario")
    must_exist("assets/scenarios/cbs_lite_demo/map.json", "K: cbs_lite_demo map")
    must_contain("assets/scenarios/cbs_lite_demo/scenario.json", r'"coordination"\s*:\s*"cbs_lite"', "K: cbs demo coordination")
    must_exist("tests/integration/OsmLaneletDemoScenarioTest.cpp", "K: OsmLaneletDemoScenarioTest")
    must_exist("tests/integration/CbsLiteDemoScenarioTest.cpp", "K: CbsLiteDemoScenarioTest")

    print("--- L CMake / Domain zero Qt ---")
    must_contain("src/domain/CMakeLists.txt", r"OsmLaneletImporter", "L: CMake OsmLaneletImporter")
    must_contain("src/domain/CMakeLists.txt", r"CbsLiteCoordinator", "L: CMake CbsLiteCoordinator")
    must_contain("src/domain/CMakeLists.txt", r"BtXmlLoader", "L: CMake BtXmlLoader")
    must_contain("src/domain/CMakeLists.txt", r"target_include_directories", "L: include dirs preserved")
    must_contain("src/domain/CMakeLists.txt", r"target_link_libraries", "L: link libs preserved")
    must_contain("tests/CMakeLists.txt", r"OsmLaneletImporterTest", "L: CMake OsmLaneletImporterTest")
    must_contain("tests/CMakeLists.txt", r"CbsLiteDemoScenarioTest", "L: CMake CbsLiteDemoScenarioTest")
    domain_hits = 0
    for p in (ROOT / "src" / "domain").rglob("*"):
        if p.suffix not in (".h", ".cpp"):
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        if re.search(r"#include\s*<Qt|rclcpp", text):
            domain_hits += 1
            bad(f"L: Qt/rclcpp in {p.relative_to(ROOT)}")
    if domain_hits == 0:
        ok("L: Domain zero Qt/rclcpp")

    print("--- M docs / mutation / plan ---")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M43", "M: M43 Osm mutation")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M44", "M: M44 Spin mutation")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M45", "M: M45 RoundRobin mutation")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M46", "M: M46 CBS mutation")
    must_contain("docs/DEVELOPMENT_PLAN.md", r"\*\*10\*\*.*✅", "M: Phase 10 marked complete")
    must_contain("docs/learning-path.md", r"Phase 10.*✅", "M: learning-path Phase 10 complete")

    print("--- N Phase 7–9 regression (static) ---")
    must_exist("assets/scenarios/bt_navigation_demo/scenario.json", "N: bt_navigation_demo")
    must_exist("assets/scenarios/lane_routing_demo/scenario.json", "N: lane_routing_demo")
    must_exist("assets/scenarios/prediction_st_demo/scenario.json", "N: prediction_st_demo")
    must_exist("tests/integration/BtNavigationDemoScenarioTest.cpp", "N: BtNavigationDemoScenarioTest")
    must_exist("tests/integration/LaneRoutingDemoScenarioTest.cpp", "N: LaneRoutingDemoScenarioTest")
    must_exist("tests/domain/BehaviorTreeTest.cpp", "N: BehaviorTreeTest (Phase 9)")
    must_exist("tests/domain/LaneGraphTest.cpp", "N: LaneGraphTest (Phase 8)")
    must_exist("tests/domain/ConstantVelocityPredictorTest.cpp", "N: ConstantVelocityPredictorTest (Phase 7)")

    print("--- O verify tooling ---")
    must_exist("tools/verify_phase10_evidence.py", "O: verify_phase10_evidence.py")
    must_exist("tools/run_phase10_verify.ps1", "O: run_phase10_verify.ps1")

    print(f"\nSummary: {len(passes)} PASS, {len(fails)} FAIL")
    if len(passes) < 60:
        bad(f"verify_phase10 requires ≥60 PASS (got {len(passes)})")
    if fails:
        for item in fails:
            print(f"  - {item}")
        return 1
    print("Static Phase 10 evidence OK (≥60 PASS). Still need FleetSimTests all green.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
