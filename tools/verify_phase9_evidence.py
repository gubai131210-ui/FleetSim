#!/usr/bin/env python3
"""Phase 9 A–K static evidence audit (no Qt Build). Exit 0 if file evidence OK."""

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

    print("--- A Behavior Tree core ---")
    must_exist("src/domain/behavior/BtTypes.h", "A: BtTypes.h")
    must_exist("src/domain/behavior/BtBlackboard.cpp", "A: BtBlackboard")
    must_exist("src/domain/behavior/BtControlNodes.cpp", "A: BtControlNodes")
    must_exist("src/domain/behavior/BtDecoratorNodes.cpp", "A: BtDecoratorNodes")
    must_exist("tests/domain/BehaviorTreeTest.cpp", "A: BehaviorTreeTest")
    must_contain("src/domain/behavior/BtTypes.h", r"NodeStatus|SUCCESS|FAILURE|RUNNING", "A: NodeStatus enum")
    must_contain("tests/domain/BehaviorTreeTest.cpp", r"SequenceAllSuccessReturnsSuccess", "A: Sequence test")
    must_contain("tests/domain/BehaviorTreeTest.cpp", r"FallbackReturnsFirstSuccess", "A: Fallback test")
    must_contain("tests/domain/BehaviorTreeTest.cpp", r"RecoveryPrimaryFailThenRecoverySuccessRetriesPrimary", "A: Recovery test")
    must_contain("tests/domain/BehaviorTreeTest.cpp", r"RateDecoratorSkipsChildWithinInterval", "A: Rate decorator test")
    must_contain(
        "src/domain/behavior/BtControlNodes.cpp",
        r"Nav2 RecoveryNode|primary first",
        "A: Recovery Nav2-aligned semantics",
    )

    print("--- B FleetSim leaf nodes ---")
    must_exist("src/domain/behavior/BtFleetActionNodes.cpp", "B: BtFleetActionNodes")
    must_exist("src/domain/behavior/BtFleetConditionNodes.cpp", "B: BtFleetConditionNodes")
    must_exist("src/domain/behavior/BtSimEngineContext.cpp", "B: BtSimEngineContext")
    must_exist("tests/domain/BtFleetNodesTest.cpp", "B: BtFleetNodesTest")
    must_contain("src/domain/behavior/BtFleetActionNodes.cpp", r"planPath|PlanPath", "B: PlanPath calls planner")
    must_contain("tests/domain/BtFleetNodesTest.cpp", r"PlanPathWritesPathValidOnSuccess", "B: PlanPath test")
    must_contain("tests/domain/BtFleetNodesTest.cpp", r"FollowUntilGoalSuccessWhenGoalReached", "B: FollowUntilGoal test")
    must_contain("tests/domain/BtFleetNodesTest.cpp", r"WaitRecoveryRunsThenSucceeds", "B: WaitRecovery test")
    must_contain("tests/domain/BtFleetNodesTest.cpp", r"IsGoalUpdatedReturnsSuccessWhenGoalChanged", "B: IsGoalUpdated test")

    print("--- C BtNavigator / JSON loader ---")
    must_exist("src/domain/behavior/BtNavigator.cpp", "C: BtNavigator")
    must_exist("src/domain/behavior/BtTreeLoader.cpp", "C: BtTreeLoader")
    must_exist("assets/behavior_trees/navigate_replan_recovery.json", "C: navigate_replan_recovery.json")
    must_exist("tests/domain/BtTreeLoaderTest.cpp", "C: BtTreeLoaderTest")
    must_contain("src/domain/behavior/BtNavigator.cpp", r"loadFromJsonFile|tick\(", "C: Navigator load + tick")
    must_contain("assets/behavior_trees/navigate_replan_recovery.json", r'"type"\s*:\s*"Recovery"', "C: asset Recovery node")
    must_contain("tests/domain/BtTreeLoaderTest.cpp", r"LoadsNavigateRecoveryAsset", "C: loader asset test")

    print("--- D SimEngine / scenario wiring ---")
    must_contain("src/domain/SimEngine.cpp", r"behavior_mode_", "D: SimEngine behavior_mode_")
    must_contain("src/domain/SimEngine.cpp", r'tickBehaviorTreeForAgent', "D: tickBehaviorTreeForAgent")
    must_contain("src/domain/SimEngine.cpp", r'behavior_mode_\s*==\s*"bt"', "D: bt branch in tick")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"behavior_mode", "D: scenario behavior_mode field")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"behavior_tree_path", "D: scenario behavior_tree_path")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"replan_hz", "D: scenario replan_hz")
    must_contain("src/app/SimController.cpp", r"resolveBehaviorTreePath", "D: SimController resolves tree path")
    must_contain(
        "tests/integration/BtNavigationIntegrationTest.cpp",
        r"LegacyModeKeepsNeedsReplanPlanning",
        "D: legacy default preserved",
    )
    must_contain(
        "tests/domain/ScenarioSerializerTest.cpp",
        r"RoundTripBehaviorModeFields",
        "D: scenario round-trip behavior fields",
    )

    print("--- E ExperimentMetrics CSV ---")
    must_contain("src/domain/experiment/ExperimentMetrics.cpp", r"exportCsv", "E: exportCsv impl")
    must_contain(
        "src/domain/experiment/ExperimentMetrics.cpp",
        r"tick,cross_track,heading_error,st_ref_v,mpc_ok",
        "E: CSV column header",
    )
    must_exist("tests/domain/ExperimentMetricsExportTest.cpp", "E: ExperimentMetricsExportTest")
    must_contain("src/ui/panels/ExperimentComparePanel.cpp", r'Export CSV|exportCsvRequested', "E: ComparePanel Export CSV")
    must_contain("src/app/MonitorBridge.cpp", r"exportCurrentMetricsCsv", "E: MonitorBridge CSV export")
    must_not_contain("src/ui/panels/ControlPanel.cpp", r"Export CSV|exportCsv", "E: no CSV in ControlPanel")
    must_not_contain(
        "src/ui/dialogs/AlgorithmWorkbenchDialog.cpp",
        r"Export CSV|exportCsv",
        "E: no CSV in Workbench",
    )

    print("--- F Behavior UI IA ---")
    must_exist("src/ui/panels/BehaviorTreePanel.cpp", "F: BehaviorTreePanel")
    must_exist("src/ui/dialogs/pages/BehaviorPage.cpp", "F: BehaviorPage")
    must_contain("src/ui/dialogs/AlgorithmWorkbenchDialog.cpp", r'addTab\(behavior_page_', "F: Workbench tab 6 Behavior")
    must_contain("src/ui/MainWindow.cpp", r"BehaviorTreePanel", "F: MainWindow mounts BehaviorTreePanel")
    must_not_contain(
        "src/ui/dialogs/pages/RoutingPage.cpp",
        r"behavior_mode|BehaviorTree|BtNavigator",
        "F: RoutingPage no BT config",
    )
    must_not_contain(
        "src/ui/dialogs/pages/PlanningPage.cpp",
        r"behavior_mode|BehaviorTree",
        "F: PlanningPage no BT config",
    )
    must_not_contain(
        "src/ui/panels/LaneEditorPanel.cpp",
        r"behavior_mode|BehaviorTree",
        "F: LaneEditorPanel no BT",
    )
    must_not_contain(
        "src/ui/panels/MonitorPanel.cpp",
        r"behavior_mode|BehaviorTree",
        "F: MonitorPanel no BT forms",
    )
    behavior_panel_lines = line_count("src/ui/panels/BehaviorTreePanel.cpp")
    if behavior_panel_lines <= 300:
        ok(f"F: BehaviorTreePanel size OK ({behavior_panel_lines} lines)")
    else:
        bad(f"F: BehaviorTreePanel too large ({behavior_panel_lines} lines)")

    print("--- G bt_navigation_demo ---")
    must_exist("assets/scenarios/bt_navigation_demo/scenario.json", "G: bt_navigation_demo scenario")
    must_exist("assets/scenarios/bt_navigation_demo/map.json", "G: bt_navigation_demo map")
    must_exist(
        "assets/scenarios/bt_navigation_demo/navigate_replan_recovery.json",
        "G: bt_navigation_demo local tree",
    )
    must_contain(
        "assets/scenarios/bt_navigation_demo/scenario.json",
        r'"behavior_mode"\s*:\s*"bt"',
        "G: demo behavior_mode=bt",
    )
    must_exist("tests/integration/BtNavigationDemoScenarioTest.cpp", "G: BtNavigationDemoScenarioTest")

    print("--- H CMake / Domain zero Qt ---")
    must_contain("src/domain/CMakeLists.txt", r"behavior/BtNavigator\.cpp", "H: BtNavigator in CMake")
    must_contain("src/domain/CMakeLists.txt", r"behavior/BtTreeLoader\.cpp", "H: BtTreeLoader in CMake")
    must_contain("src/ui/CMakeLists.txt", r"BehaviorTreePanel\.cpp", "H: BehaviorTreePanel in UI CMake")
    must_contain("src/domain/CMakeLists.txt", r"target_include_directories", "H: include dirs preserved")
    must_contain("src/domain/CMakeLists.txt", r"target_link_libraries", "H: link libs preserved")
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

    print("--- I docs ADR / plan ---")
    must_exist("docs/decisions/020-behavior-tree-mvp.md", "I: ADR-020")
    must_exist("docs/decisions/021-behavior-ui-ia.md", "I: ADR-021")
    must_contain("docs/decisions/020-behavior-tree-mvp.md", r"已接受", "I: ADR-020 accepted")
    must_contain("docs/decisions/021-behavior-ui-ia.md", r"已接受", "I: ADR-021 accepted")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M40", "I: M40")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M41", "I: M41")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M42", "I: M42")
    must_contain("docs/DEVELOPMENT_PLAN.md", r"\*\*9\*\*.*✅", "I: Phase 9 marked complete")

    print("--- J tests registered ---")
    for test in (
        "BehaviorTreeTest",
        "BtTreeLoaderTest",
        "BtFleetNodesTest",
        "BtNavigationIntegrationTest",
        "BtNavigationDemoScenarioTest",
        "ExperimentMetricsExportTest",
    ):
        must_contain("tests/CMakeLists.txt", test, f"J: {test} in CMake")

    print("--- Phase 8 regression (static) ---")
    must_exist("tests/integration/LaneRoutingDemoScenarioTest.cpp", "P8: LaneRoutingDemoScenarioTest")
    must_exist("assets/scenarios/lane_routing_demo/scenario.json", "P8: lane_routing_demo")
    must_exist("src/domain/map/LaneGraph.cpp", "P8: LaneGraph still present")

    print("--- Phase 7 regression (static) ---")
    must_exist("tests/domain/ConstantVelocityPredictorTest.cpp", "P7: ConstantVelocityPredictorTest")
    must_exist("tests/integration/ExperimentCompareIntegrationTest.cpp", "P7: ExperimentCompareIntegrationTest")
    must_exist("assets/scenarios/prediction_st_demo/scenario.json", "P7: prediction_st_demo")

    print("--- K verify tooling ---")
    must_exist("tools/verify_phase9_evidence.py", "K: verify_phase9_evidence.py")
    must_exist("tools/run_phase9_verify.ps1", "K: run_phase9_verify.ps1")

    print("--- summary ---")
    print(f"passes={len(passes)} fails={len(fails)}")
    if fails:
        print("USER ACTION: fix FAIL items; then Build FleetSimTests for runtime green.")
        return 1
    print("Static Phase 9 evidence OK. Still need FleetSimTests all green (ASCII or user Qt).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
