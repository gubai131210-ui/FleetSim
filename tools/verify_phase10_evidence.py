#!/usr/bin/env python3
"""Phase 10 static evidence audit (Session 0 skeleton; Session 7 full ≥60 checks)."""

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


def main() -> int:
    print(f"ROOT={ROOT}")
    print("--- Session 0 skeleton (full audit Session 7) ---")

    print("--- ADR ---")
    must_exist("docs/decisions/022-osm-lanelet-import.md", "ADR-022")
    must_exist("docs/decisions/023-bt-xml-motion-recovery.md", "ADR-023")
    must_exist("docs/decisions/024-cbs-lite.md", "ADR-024")
    must_exist("docs/decisions/025-multi-bt-ui-ia.md", "ADR-025")

    print("--- Domain Phase 10 stubs ---")
    must_exist("src/domain/map/OsmLaneletImporter.h", "OsmLaneletImporter.h")
    must_exist("src/domain/behavior/BtXmlLoader.h", "BtXmlLoader.h")
    must_exist("src/domain/behavior/BtMotionRecoveryNodes.h", "BtMotionRecoveryNodes.h")
    must_exist("src/domain/behavior/MultiBtNavigator.h", "MultiBtNavigator.h")
    must_exist("src/domain/collision/CbsLiteCoordinator.h", "CbsLiteCoordinator.h")
    must_contain("src/domain/behavior/BtControlNodes.h", r"BtRoundRobinNode", "RoundRobin node")
    must_contain("src/domain/behavior/BtControlNodes.h", r"BtReactiveFallbackNode", "ReactiveFallback node")

    print("--- Tests ---")
    must_exist("tests/domain/OsmLaneletImporterTest.cpp", "OsmLaneletImporterTest")
    must_exist("tests/domain/BtXmlLoaderTest.cpp", "BtXmlLoaderTest")
    must_exist("tests/domain/BtMotionRecoveryTest.cpp", "BtMotionRecoveryTest")
    must_exist("tests/domain/CbsLiteCoordinatorTest.cpp", "CbsLiteCoordinatorTest")
    must_exist("tests/integration/MultiBtNavigationTest.cpp", "MultiBtNavigationTest")

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
