#!/usr/bin/env python3
"""Phase 6 A–J static evidence audit (no Qt Build). Exit 0 if file evidence OK."""

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


def main() -> int:
    print(f"ROOT={ROOT}")

    print("--- A/B MPC ---")
    must_exist("src/domain/control/MpcLateralTracker.cpp", "A: MpcLateralTracker.cpp")
    must_exist("src/domain/control/DenseQpSolver.cpp", "B: DenseQpSolver.cpp")
    must_contain(
        "src/domain/control/MpcLateralTracker.cpp",
        r"DenseQpSolver|last_prediction_norm_",
        "A: prediction / QP path",
    )
    must_contain("src/domain/SimEngine.cpp", r"tracker_kind_ == \"mpc\"|\"mpc\"", "A: SimEngine mpc branch")

    print("--- C/D ST-Graph ---")
    must_exist("src/core/types/SpeedProfile.h", "C: SpeedProfile.h")
    must_exist("src/domain/planning/StGraphSpeedPlanner.cpp", "C: StGraphSpeedPlanner.cpp")
    must_contain(
        "src/domain/planning/StGraphSpeedPlanner.cpp",
        r"buildOccupancies|PeerTrajectory|peers",
        "C: peers consumed",
    )
    must_contain("src/domain/SimEngine.cpp", r"st_graph|refreshSpeedProfiles|collectPeersFor", "D: ST wired")
    must_contain("src/domain/SimEngine.cpp", r"setSpeedProfile", "D: MPC setSpeedProfile")
    must_contain("src/domain/SimEngine.cpp", r"speed_scale", "D: TimeWindow scale retained")

    print("--- E scenario/Dialog ---")
    must_contain("src/ui/dialogs/PlannerTrackerDialog.cpp", r"mpc|st_graph", "E: Dialog mpc/st")
    must_contain("src/domain/scenario/ScenarioLoader.h", r"speed_planner", "E: speed_planner field")
    must_exist("assets/scenarios/mpc_st_demo/scenario.json", "E: mpc_st_demo asset")

    print("--- F Monitor ---")
    must_contain("src/ui/panels/MonitorPanel.cpp", r"ST ref|st_speed|st_reference", "F: ST speed curve")
    must_not_contain(
        "src/ui/panels/ControlPanel.cpp",
        r"st_graph|MpcLateral|Speed planner",
        "F: ControlPanel not dumped",
    )

    print("--- G CMake / Domain Qt ---")
    must_contain("src/domain/CMakeLists.txt", r"MpcLateralTracker\.cpp", "G: MPC in CMake")
    must_contain("src/domain/CMakeLists.txt", r"StGraphSpeedPlanner\.cpp", "G: ST in CMake")
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
    must_exist("docs/decisions/014-linear-mpc.md", "H: ADR-014")
    must_exist("docs/decisions/015-st-graph-speed.md", "H: ADR-015")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M31", "H: M31")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M32", "H: M32")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M33", "H: M33")
    must_contain("docs/DEVELOPMENT_PLAN.md", r"\*\*6\*\*.*✅", "H: Phase 6 marked")

    print("--- I tests ---")
    must_exist("tests/domain/MpcLateralTrackerTest.cpp", "I: MpcLateralTrackerTest")
    must_exist("tests/domain/StGraphSpeedPlannerTest.cpp", "I: StGraphSpeedPlannerTest")
    must_exist("tests/domain/MpcVsStanleyCompareTest.cpp", "I: MpcVsStanleyCompareTest")
    must_exist("tests/domain/StGraphSimEngineWiringTest.cpp", "I: StGraphSimEngineWiringTest")
    must_contain(
        "tests/domain/StGraphSimEngineWiringTest.cpp",
        r"WithPeersProfileDiffersFromEmptyPeers",
        "I: multi-agent peer contract",
    )

    print("--- summary ---")
    print(f"passes={len(passes)} fails={len(fails)}")
    if fails:
        print("USER ACTION: fix FAIL items; then Build FleetSimTests for runtime green.")
        return 1
    print("Static Phase 6 evidence OK. Still need FleetSimTests all green (ASCII or user Qt).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
