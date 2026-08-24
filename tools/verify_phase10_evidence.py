#!/usr/bin/env python3
"""Phase 10 static evidence audit (Session 6 UI + Session 7 full ≥60 checks)."""

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

    print("--- ADR ---")
    must_exist("docs/decisions/022-osm-lanelet-import.md", "ADR-022")
    must_exist("docs/decisions/023-bt-xml-motion-recovery.md", "ADR-023")
    must_exist("docs/decisions/024-cbs-lite.md", "ADR-024")
    must_exist("docs/decisions/025-multi-bt-ui-ia.md", "ADR-025")

    print("--- Domain ---")
    must_exist("src/domain/map/OsmLaneletImporter.h", "OsmLaneletImporter.h")
    must_exist("src/domain/behavior/BtXmlLoader.h", "BtXmlLoader.h")
    must_exist("src/domain/behavior/BtMotionRecoveryNodes.h", "BtMotionRecoveryNodes.h")
    must_exist("src/domain/behavior/MultiBtNavigator.h", "MultiBtNavigator.h")
    must_exist("src/domain/collision/CbsLiteCoordinator.h", "CbsLiteCoordinator.h")
    must_contain("src/domain/behavior/BtControlNodes.h", r"BtRoundRobinNode", "RoundRobin node")
    must_contain(
        "src/domain/behavior/BtControlNodes.h", r"BtReactiveFallbackNode", "ReactiveFallback node"
    )

    print("--- UI Phase 10 four-piece ---")
    must_exist("src/ui/panels/OsmImportPanel.h", "OsmImportPanel")
    must_exist("src/ui/panels/MultiAgentBehaviorPanel.h", "MultiAgentBehaviorPanel")
    must_exist("src/ui/dialogs/pages/MapImportPage.h", "MapImportPage")
    must_exist("src/ui/dialogs/pages/BehaviorXmlPage.h", "BehaviorXmlPage")
    must_contain("src/ui/dialogs/AlgorithmWorkbenchDialog.cpp", r"Map Import", "Workbench Tab 7")
    must_contain("src/ui/dialogs/AlgorithmWorkbenchDialog.cpp", r"Behavior XML", "Workbench Tab 8")
    must_contain("src/ui/dialogs/pages/CoordinationPage.cpp", r"cbs_lite", "Coordination cbs_lite")
    must_contain("src/ui/CMakeLists.txt", r"OsmImportPanel", "CMake OsmImportPanel")
    must_contain("src/ui/CMakeLists.txt", r"MapImportPage", "CMake MapImportPage")

    print("--- UI negative audit (ADR-025) ---")
    must_not_contain(
        "src/ui/panels/MapEditorPanel.cpp", r"osm|OsmImport|\.osm", "MapEditorPanel no OSM pollution"
    )
    must_not_contain(
        "src/ui/panels/LaneEditorPanel.cpp", r"osm|OsmImport|\.osm", "LaneEditorPanel no OSM pollution"
    )
    must_not_contain(
        "src/ui/dialogs/pages/BehaviorPage.cpp",
        r"bt_format|RoundRobin|spin_rad|backup_dist",
        "BehaviorPage no XML/recovery pollution",
    )
    must_not_contain(
        "src/ui/dialogs/pages/RoutingPage.cpp",
        r"cbs_lite|osm_path",
        "RoutingPage no CBS/OSM pollution",
    )
    must_not_contain(
        "src/ui/dialogs/pages/CoordinationPage.cpp",
        r"behavior_tree|osm",
        "CoordinationPage no BT/OSM pollution",
    )

    print("--- UI line limits ---")
    must_under_lines("src/ui/panels/OsmImportPanel.cpp", 300, "OsmImportPanel.cpp ≤300")
    must_under_lines("src/ui/panels/MultiAgentBehaviorPanel.cpp", 300, "MultiAgentBehaviorPanel.cpp ≤300")
    must_under_lines("src/ui/dialogs/pages/MapImportPage.cpp", 300, "MapImportPage.cpp ≤300")
    must_under_lines("src/ui/dialogs/pages/BehaviorXmlPage.cpp", 300, "BehaviorXmlPage.cpp ≤300")

    print("--- Tests ---")
    must_exist("tests/domain/OsmLaneletImporterTest.cpp", "OsmLaneletImporterTest")
    must_exist("tests/domain/BtXmlLoaderTest.cpp", "BtXmlLoaderTest")
    must_exist("tests/domain/BtMotionRecoveryTest.cpp", "BtMotionRecoveryTest")
    must_exist("tests/domain/CbsLiteCoordinatorTest.cpp", "CbsLiteCoordinatorTest")
    must_exist("tests/integration/MultiBtNavigationTest.cpp", "MultiBtNavigationTest")

    print("--- Scenario fields ---")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"map_source", "scenario map_source")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"bt_format", "scenario bt_format")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"cbs_max_depth", "scenario cbs_max_depth")

    print("--- Assets ---")
    must_exist("assets/maps/teaching_lanelet_subset.osm", "teaching OSM")
    must_exist("assets/behavior_trees/navigate_spin_backup_recovery.xml", "Nav2-style BT XML")

    print("--- CMake ---")
    must_contain("src/domain/CMakeLists.txt", r"OsmLaneletImporter", "CMake OsmLaneletImporter")
    must_contain("src/domain/CMakeLists.txt", r"CbsLiteCoordinator", "CMake CbsLiteCoordinator")
    must_contain("tests/CMakeLists.txt", r"OsmLaneletImporterTest", "CMake OsmLaneletImporterTest")

    print(f"\nSummary: {len(passes)} PASS, {len(fails)} FAIL")
    if fails:
        for item in fails:
            print(f"  - {item}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
