#!/usr/bin/env python3
"""Phase 5 A–J static evidence audit (no Qt Build). Exit 0 if file evidence OK."""

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

    print("--- A Hybrid A* ---")
    must_exist("src/domain/planning/HybridAStarPlanner.cpp", "A: HybridAStarPlanner.cpp")
    must_exist("src/domain/planning/DubinsPath.cpp", "A: DubinsPath.cpp")
    must_contain(
        "src/domain/planning/HybridAStarPlanner.cpp",
        r"tan\(steer\)|std::tan\(steer\)",
        "A: bicycle steering expansion",
    )
    must_contain(
        "src/domain/planning/HybridAStarPlanner.cpp",
        r"DubinsPath",
        "A: Dubins analytic expansion",
    )

    print("--- B planner switch ---")
    must_contain("src/domain/SimEngine.cpp", r"hybrid_astar", "B: SimEngine hybrid branch")
    must_contain(
        "src/domain/scenario/ScenarioLoader.h",
        r"planner",
        "B: SimulationConfig.planner",
    )
    must_exist("assets/scenarios/hybrid_narrow_turn/scenario.json", "B: hybrid_narrow_turn asset")

    print("--- C Stanley ---")
    must_contain(
        "src/domain/control/StanleyTracker.cpp",
        r"atan2\(k_gain_|std::atan2\(k_gain_",
        "C: Stanley formula term",
    )
    must_exist("tests/domain/StanleyTrackerTest.cpp", "C: StanleyTrackerTest")

    print("--- D tracker inject ---")
    must_contain("src/domain/SimEngine.cpp", r"stanley", "D: SimEngine stanley branch")
    must_contain("src/domain/SimEngine.cpp", r"pure_pursuit", "D: Pure Pursuit retained")

    print("--- E Priority ---")
    must_exist(
        "src/domain/collision/PriorityPathCoordinator.cpp",
        "E: PriorityPathCoordinator",
    )
    must_contain(
        "src/domain/SimEngine.cpp",
        r"replanFleetWithPriorityCoordination|paintPathOccupied",
        "E: priority replan wired",
    )
    must_contain(
        "src/domain/SimEngine.cpp",
        r"collision_\.tick",
        "E: TimeWindow tick retained",
    )

    print("--- F UI ---")
    must_exist("src/ui/dialogs/PlannerTrackerDialog.cpp", "F: PlannerTrackerDialog")
    must_not_contain(
        "src/ui/panels/ControlPanel.cpp",
        r"hybrid_astar|Stanley|PlannerTracker",
        "F: ControlPanel not dumped",
    )
    must_contain(
        "src/ui/panels/MonitorPanel.cpp",
        r"heading",
        "F: Monitor heading error",
    )

    print("--- G CMake / Domain Qt ---")
    must_contain(
        "src/domain/CMakeLists.txt",
        r"HybridAStarPlanner\.cpp",
        "G: Hybrid in Domain CMake",
    )
    must_contain(
        "src/domain/CMakeLists.txt",
        r"target_include_directories",
        "G: include dirs kept",
    )
    must_contain(
        "src/domain/CMakeLists.txt",
        r"target_link_libraries",
        "G: link libs kept",
    )
    # Spot-check Domain sources for Qt
    domain_hits = 0
    for pattern in ("*.h", "*.cpp"):
        for p in (ROOT / "src" / "domain").rglob(pattern):
            text = p.read_text(encoding="utf-8", errors="replace")
            if re.search(r"#include\s*<Qt|rclcpp", text):
                domain_hits += 1
                bad(f"G: Qt/rclcpp in {p.relative_to(ROOT)}")
    if domain_hits == 0:
        ok("G: Domain zero Qt/rclcpp (spot scan)")

    print("--- H docs ---")
    must_exist("docs/decisions/011-hybrid-astar.md", "H: ADR-011")
    must_exist("docs/decisions/012-stanley-tracker.md", "H: ADR-012")
    must_exist("docs/decisions/013-priority-or-cbs-lite.md", "H: ADR-013")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M28", "H: MUTATION M28")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M29", "H: MUTATION M29")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M30", "H: MUTATION M30")
    must_contain("docs/DEVELOPMENT_PLAN.md", r"\*\*5\*\*.*✅", "H: Phase 5 marked")

    print("--- I tests ---")
    must_exist("tests/domain/HybridAStarPlannerTest.cpp", "I: HybridAStarPlannerTest")
    must_exist("tests/domain/StanleyTrackerTest.cpp", "I: StanleyTrackerTest")
    must_exist("tests/domain/PlannerSwitchIntegrationTest.cpp", "I: PlannerSwitchIntegrationTest")
    must_exist("tests/domain/PriorityCoordinatorTest.cpp", "I: PriorityCoordinatorTest")

    print("--- summary ---")
    print(f"passes={len(passes)} fails={len(fails)}")
    if fails:
        print("USER ACTION: fix FAIL items; then Build FleetSimTests locally for runtime green.")
        return 1
    print("Static Phase 5 evidence OK. Still need user-local FleetSimTests all green.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
