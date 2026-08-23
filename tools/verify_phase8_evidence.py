#!/usr/bin/env python3
"""Phase 8 A–K static evidence audit (no Qt Build). Exit 0 if file evidence OK."""

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


def must_contain(rel: str, pattern: str, label: str) -> None:
    text = read(rel)
    if not text:
        return
    if re.search(pattern, text, re.MULTILINE | re.DOTALL):
        ok(label)
    else:
        bad(f"{label} — not in {rel}: {pattern}")


def must_exist(rel: str, label: str) -> None:
    if (ROOT / rel).exists():
        ok(label)
    else:
        bad(label)


def must_not_contain(rel: str, pattern: str, label: str) -> None:
    text = read(rel)
    if not text:
        return
    if re.search(pattern, text):
        bad(f"{label} — forbidden in {rel}: {pattern}")
    else:
        ok(label)


def line_count(rel: str) -> int:
    path = ROOT / rel
    if not path.exists():
        return 0
    return len(path.read_text(encoding="utf-8", errors="replace").splitlines())


def main() -> int:
    print(f"ROOT={ROOT}")

    print("--- A LaneGraph ---")
    must_exist("src/domain/map/LaneGraph.cpp", "A: LaneGraph.cpp")
    must_exist("src/domain/map/LaneTypes.h", "A: LaneTypes.h")
    must_exist("tests/domain/LaneGraphTest.cpp", "A: LaneGraphTest")
    must_contain("src/domain/map/LaneGraph.cpp", r"shortestPath|best_cost|Dijkstra|priority_queue", "A: Dijkstra impl")
    must_contain("tests/domain/LaneGraphTest.cpp", r"ShortestPathForkChoosesShorterRoute", "A: fork test")

    print("--- B LaneRouter ---")
    must_exist("src/domain/planning/LaneRouter.cpp", "B: LaneRouter.cpp")
    must_exist("tests/domain/LaneRouterTest.cpp", "B: LaneRouterTest")
    must_contain("src/domain/planning/LaneRouter.cpp", r"centerlinePath|shortestPath", "B: router pipeline")

    print("--- C First/Last Mile ---")
    must_exist("tests/integration/FirstLastMileIntegrationTest.cpp", "C: FirstLastMileIntegrationTest")
    must_contain("src/domain/SimEngine.cpp", r"planHybridPathForAgent", "C: hybrid planner")
    must_contain(
        "tests/integration/FirstLastMileIntegrationTest.cpp",
        r"ThreeRoutingModesProduceDistinctPaths",
        "C: three modes distinct",
    )

    print("--- D SimEngine routing_mode ---")
    must_contain("src/domain/SimEngine.cpp", r'routing_mode_\s*==\s*"lane_graph"', "D: lane_graph branch")
    must_contain("src/domain/SimEngine.cpp", r'routing_mode_\s*==\s*"hybrid"', "D: hybrid branch")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"routing_mode", "D: scenario routing_mode")
    must_contain("src/app/SimController.cpp", r"setRoutingMode", "D: SimController applies routing")

    print("--- E MapSerializer lanes ---")
    must_contain("src/domain/map/MapData.h", r"LaneMapData\s+lanes", "E: MapDocument.lanes")
    must_contain("src/domain/map/MapSerializer.cpp", r"parseLanesJson|lanesToJson", "E: lanes serialize")
    must_exist("tests/domain/MapSerializerLaneTest.cpp", "E: MapSerializerLaneTest")

    print("--- F UI IA ---")
    must_exist("src/ui/panels/LaneEditorPanel.cpp", "F: LaneEditorPanel")
    must_exist("src/ui/dialogs/pages/RoutingPage.cpp", "F: RoutingPage")
    must_exist("src/ui/graphics/LaneGraphicsItem.cpp", "F: LaneGraphicsItem")
    must_not_contain(
        "src/ui/panels/MapEditorPanel.cpp",
        r"lane|Lane|routing_mode",
        "F: MapEditorPanel no lane/routing",
    )
    must_not_contain(
        "src/ui/dialogs/pages/SpeedPage.cpp",
        r"routing_mode|LaneEditor",
        "F: SpeedPage no routing",
    )
    must_not_contain(
        "src/ui/dialogs/pages/PlanningPage.cpp",
        r"routing_mode|LaneEditor",
        "F: PlanningPage no routing",
    )
    must_not_contain(
        "src/ui/panels/ExperimentComparePanel.cpp",
        r"routing_mode|lane_snap",
        "F: ComparePanel no routing",
    )
    map_editor_lines = line_count("src/ui/panels/MapEditorPanel.cpp")
    if map_editor_lines <= 60:
        ok(f"F: MapEditorPanel baseline unchanged ({map_editor_lines} lines)")
    else:
        bad(f"F: MapEditorPanel grew too much ({map_editor_lines} lines)")
    must_contain("src/ui/dialogs/AlgorithmWorkbenchDialog.cpp", r'addTab\(routing_page_', "F: Workbench tab 5")

    print("--- G lane_routing_demo ---")
    must_exist("assets/scenarios/lane_routing_demo/map.json", "G: lane_routing_demo map")
    must_exist("assets/scenarios/lane_routing_demo/scenario.json", "G: lane_routing_demo scenario")
    must_contain(
        "assets/scenarios/lane_routing_demo/map.json",
        r'"nodes"\s*:',
        "G: lanes nodes in demo map",
    )
    must_contain(
        "assets/scenarios/lane_routing_demo/scenario.json",
        r"hybrid|routing_mode",
        "G: demo routing_mode",
    )
    must_exist("tests/integration/LaneRoutingDemoScenarioTest.cpp", "G: LaneRoutingDemoScenarioTest")

    print("--- H CMake / Domain Qt ---")
    must_contain("src/domain/CMakeLists.txt", r"LaneGraph\.cpp", "H: LaneGraph in CMake")
    must_contain("src/domain/CMakeLists.txt", r"LaneRouter\.cpp", "H: LaneRouter in CMake")
    must_contain("src/domain/CMakeLists.txt", r"target_include_directories", "H: include dirs")
    must_contain("src/domain/CMakeLists.txt", r"target_link_libraries", "H: link libs")
    domain_hits = 0
    for p in (ROOT / "src" / "domain").rglob("*"):
        if p.suffix not in (".h", ".cpp"):
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        if re.search(r"#include\s*<Qt|rclcpp", text):
            domain_hits += 1
            bad(f"H: Qt/rclcpp in {p.relative_to(ROOT)}")
    if domain_hits == 0:
        ok("H: Domain zero Qt/rclcpp")

    print("--- I docs ADR ---")
    must_exist("docs/decisions/018-lane-graph-routing.md", "I: ADR-018")
    must_exist("docs/decisions/019-lane-editor-ui.md", "I: ADR-019")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M37", "I: M37")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M38", "I: M38")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M39", "I: M39")
    must_contain("docs/DEVELOPMENT_PLAN.md", r"\*\*8\*\*.*✅", "I: Phase 8 marked")

    print("--- J tests registered ---")
    for test in (
        "LaneGraphTest",
        "LaneRouterTest",
        "MapSerializerLaneTest",
        "FirstLastMileIntegrationTest",
        "LaneRoutingDemoScenarioTest",
    ):
        must_contain("tests/CMakeLists.txt", test, f"J: {test} in CMake")

    print("--- Phase 7 regression (static) ---")
    must_exist("tests/domain/ConstantVelocityPredictorTest.cpp", "P7: ConstantVelocityPredictorTest")
    must_exist("tests/integration/ExperimentCompareIntegrationTest.cpp", "P7: ExperimentCompareIntegrationTest")
    must_exist("src/ui/panels/ExperimentComparePanel.cpp", "P7: ExperimentComparePanel")
    must_exist("tests/domain/MpcLateralTrackerTest.cpp", "P6: MpcLateralTrackerTest")
    must_exist("tests/domain/HybridAStarPlannerTest.cpp", "P5: HybridAStarPlannerTest")

    print("--- K verify tooling ---")
    must_exist("tools/verify_phase8_evidence.py", "K: verify_phase8_evidence.py")
    must_exist("tools/run_phase8_verify.ps1", "K: run_phase8_verify.ps1")

    print("--- summary ---")
    print(f"passes={len(passes)} fails={len(fails)}")
    if fails:
        print("USER ACTION: fix FAIL items; then Build FleetSimTests for runtime green.")
        return 1
    print("Static Phase 8 evidence OK. Still need FleetSimTests all green (ASCII or user Qt).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
