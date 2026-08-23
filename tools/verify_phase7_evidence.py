#!/usr/bin/env python3
"""Phase 7 A–J static evidence audit (no Qt Build). Exit 0 if file evidence OK."""

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

    print("--- A ConstantVelocityPredictor ---")
    must_exist("src/domain/prediction/ConstantVelocityPredictor.cpp", "A: ConstantVelocityPredictor.cpp")
    must_exist("tests/domain/ConstantVelocityPredictorTest.cpp", "A: ConstantVelocityPredictorTest")
    must_contain(
        "src/domain/prediction/ConstantVelocityPredictor.cpp",
        r"cos\(current\.theta\)|cos_theta",
        "A: CV extrapolation uses heading",
    )

    print("--- B ST + prediction wiring ---")
    must_contain("src/domain/SimEngine.cpp", r"constant_velocity|prediction_kind_", "B: SimEngine prediction")
    must_contain("src/domain/SimEngine.cpp", r"ConstantVelocityPredictor|cv_predictor", "B: collectPeers uses predictor")
    must_exist("tests/domain/StGraphWithPredictionTest.cpp", "B: StGraphWithPredictionTest")
    must_contain(
        "tests/domain/StGraphSimEngineWiringTest.cpp",
        r"PredictionConstantVelocityChangesStProfile",
        "B: wiring prediction differs ST",
    )

    print("--- C ExperimentMetrics ---")
    must_exist("src/domain/experiment/ExperimentMetrics.cpp", "C: ExperimentMetrics.cpp")
    must_exist("tests/domain/ExperimentMetricsTest.cpp", "C: ExperimentMetricsTest")

    print("--- D UI workbench + compare ---")
    must_exist("src/ui/dialogs/AlgorithmWorkbenchDialog.cpp", "D: AlgorithmWorkbenchDialog")
    for page in ("PlanningPage", "ControlPage", "SpeedPage", "CoordinationPage"):
        must_exist(f"src/ui/dialogs/pages/{page}.cpp", f"D: {page}")
    must_exist("src/ui/panels/ExperimentComparePanel.cpp", "D: ExperimentComparePanel")
    must_not_contain(
        "src/ui/panels/MonitorPanel.cpp",
        r"ExperimentCompare|Capture Baseline|Baseline",
        "D: MonitorPanel no compare UI",
    )
    ptd_lines = line_count("src/ui/dialogs/PlannerTrackerDialog.cpp")
    if ptd_lines <= 15:
        ok(f"D: PlannerTrackerDialog thin wrapper ({ptd_lines} lines)")
    else:
        bad(f"D: PlannerTrackerDialog too long ({ptd_lines} lines)")

    print("--- E scenarios ---")
    must_exist("assets/scenarios/prediction_st_demo/scenario.json", "E: prediction_st_demo")
    must_contain(
        "assets/scenarios/prediction_st_demo/scenario.json",
        r"constant_velocity|st_graph",
        "E: prediction_st_demo fields",
    )

    print("--- F scenario.prediction read ---")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"prediction", "F: SimulationConfig.prediction")
    must_contain("src/app/SimController.cpp", r"setPredictionKind", "F: SimController applies prediction")

    print("--- G CMake / Domain Qt ---")
    must_contain("src/domain/CMakeLists.txt", r"ConstantVelocityPredictor\.cpp", "G: predictor in CMake")
    must_contain("src/domain/CMakeLists.txt", r"ExperimentMetrics\.cpp", "G: metrics in CMake")
    must_contain("src/domain/CMakeLists.txt", r"target_include_directories", "G: include dirs")
    must_contain("src/domain/CMakeLists.txt", r"target_link_libraries", "G: link libs")
    domain_hits = 0
    for pattern in ("*.h", "*.cpp"):
        for p in (ROOT / "src" / "domain").rglob(pattern):
            text = p.read_text(encoding="utf-8", errors="replace")
            if re.search(r"#include\s*<Qt|rclcpp", text):
                domain_hits += 1
                bad(f"G: Qt/rclcpp in {p.relative_to(ROOT)}")
    if domain_hits == 0:
        ok("G: Domain zero Qt/rclcpp")

    print("--- H docs ---")
    must_exist("docs/decisions/016-constant-velocity-prediction.md", "H: ADR-016")
    must_exist("docs/decisions/017-algorithm-workbench-ui.md", "H: ADR-017")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M34", "H: M34")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M35", "H: M35")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M36", "H: M36")
    must_contain("docs/DEVELOPMENT_PLAN.md", r"\*\*7\*\*.*✅", "H: Phase 7 marked")

    print("--- I tests ---")
    must_exist("tests/integration/ExperimentCompareIntegrationTest.cpp", "I: ExperimentCompareIntegrationTest")
    must_contain("tests/CMakeLists.txt", r"ConstantVelocityPredictorTest", "I: Predictor test registered")
    must_contain("tests/CMakeLists.txt", r"ExperimentMetricsTest", "I: Metrics test registered")

    print("--- Phase 6 regression (static) ---")
    must_exist("tests/domain/MpcLateralTrackerTest.cpp", "P6: MpcLateralTrackerTest")
    must_exist("tests/domain/StGraphSimEngineWiringTest.cpp", "P6: StGraphSimEngineWiringTest")
    must_exist("tests/domain/PriorityCoordinatorTest.cpp", "P5: PriorityCoordinatorTest")
    must_exist("tests/domain/HybridAStarPlannerTest.cpp", "P5: HybridAStarPlannerTest")

    print("--- summary ---")
    print(f"passes={len(passes)} fails={len(fails)}")
    if fails:
        print("USER ACTION: fix FAIL items; then Build FleetSimTests for runtime green.")
        return 1
    print("Static Phase 7 evidence OK. Still need FleetSimTests all green (ASCII or user Qt).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
