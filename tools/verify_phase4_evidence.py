#!/usr/bin/env python3
"""Phase 4 A–J static evidence audit (no Qt Build). Exit 0 if file evidence OK."""

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
    if re.search(pattern, text, re.MULTILINE):
        ok(label)
    else:
        bad(f"{label} — not in {rel}: {pattern}")


def must_exist(rel: str, label: str) -> None:
    if (ROOT / rel).exists():
        ok(label)
    else:
        bad(label)


def main() -> int:
    print(f"ROOT={ROOT}")

    print("--- A Bicycle kinematics ---")
    must_contain(
        "src/domain/vehicle/BicycleModel.cpp",
        r"std::tan\(steering_angle\)",
        "A: Bicycle uses (v/L)*tan(delta)",
    )
    bike = read("src/domain/vehicle/BicycleModel.cpp")
    if bike and "angular_velocity" in bike and "Ignores" in bike:
        ok("A: Bicycle ignores angular_velocity")
    elif bike:
        bad("A: Bicycle should document ignoring angular_velocity")
    diff = read("src/domain/vehicle/DiffDriveModel.cpp")
    if diff and "tan(" not in diff:
        ok("A: DiffDrive is not bicycle rebrand")
    elif diff:
        bad("A: DiffDrive must not use tan(steering)")

    print("--- B scenario + bicycle_demo ---")
    must_exist("assets/scenarios/bicycle_demo/scenario.json", "B: bicycle_demo scenario.json")
    must_contain(
        "assets/scenarios/bicycle_demo/scenario.json",
        r'"model"\s*:\s*"bicycle"',
        "B: scenario model=bicycle",
    )
    must_contain(
        "src/domain/scenario/ScenarioLoader.h",
        r"wheelbase_m",
        "B: VehicleConfig.wheelbase_m",
    )
    mw = read("src/ui/MainWindow.cpp")
    if "syncSettingsFromScenario" in mw:
        ok("B: Open Project syncs settings FROM scenario")
    else:
        bad("B: missing syncSettingsFromScenario")
    m = re.search(
        r"void MainWindow::applyProjectToSimulation\(\)\s*\{(.*?)\nvoid ",
        mw,
        re.DOTALL,
    )
    body = m.group(1) if m else ""
    if body and "current_settings_.vehicle_model" in body:
        bad("B: applyProjectToSimulation still overwrites model from settings")
    else:
        ok("B: applyProjectToSimulation preserves scenario models")

    print("--- C ControlCommand ---")
    must_contain(
        "src/core/types/ControlCommand.h",
        r"steering_angle",
        "C: ControlCommand.steering_angle",
    )

    print("--- D Pure Pursuit steering ---")
    must_contain(
        "src/domain/control/SteeringAdapter.cpp",
        r"atan\(curvature \* wheelbase_m\)",
        "D: SteeringAdapter delta=atan(kappa*L)",
    )
    must_exist("tests/domain/SteeringAdapterTest.cpp", "D: SteeringAdapterTest")
    must_contain(
        "src/domain/control/PurePursuitTracker.cpp",
        r"SteeringAdapter",
        "D: PurePursuit uses SteeringAdapter",
    )

    print("--- E CMake export + smoke ---")
    must_exist("cmake/FleetSimDomainConfig.cmake.in", "E: FleetSimDomainConfig.cmake.in")
    must_contain("src/domain/CMakeLists.txt", r"install\(EXPORT", "E: Domain install EXPORT")
    must_contain(
        "src/domain/CMakeLists.txt",
        r"target_include_directories",
        "E: Domain target_include_directories kept",
    )
    must_contain(
        "src/domain/CMakeLists.txt",
        r"target_link_libraries",
        "E: Domain target_link_libraries kept",
    )
    must_exist("examples/domain_smoke/main.cpp", "E: domain_smoke main")
    must_exist("docs/domain_export.md", "E: domain_export.md")

    print("--- F ROS2 optional ---")
    must_contain(
        "CMakeLists.txt",
        r"option\(FLEETSIM_BUILD_ROS2\s+\"Build ROS2 bridge\"\s+OFF\)",
        "F: FLEETSIM_BUILD_ROS2 default OFF",
    )
    must_exist("bridges/ros2/CMakeLists.txt", "F: bridges/ros2")
    qt_hits = []
    for p in (ROOT / "src/domain").rglob("*"):
        if p.suffix.lower() not in {".h", ".hpp", ".cpp", ".cc"}:
            continue
        t = p.read_text(encoding="utf-8", errors="replace")
        if re.search(r"#include\s*<Q|#include\s*<rclcpp", t):
            qt_hits.append(str(p.relative_to(ROOT)))
    if qt_hits:
        bad(f"F: Domain has Qt/rclcpp includes: {qt_hits}")
    else:
        ok("F: Domain has zero Qt/rclcpp includes")

    print("--- G SettingsDialog ---")
    must_exist("src/ui/dialogs/SettingsDialog.h", "G: SettingsDialog.h")
    must_contain("src/ui/MainWindow.cpp", r"SettingsDialog", "G: MainWindow mounts SettingsDialog")
    for p in (ROOT / "src/ui/panels").glob("ControlPanel.*"):
        t = p.read_text(encoding="utf-8", errors="replace")
        if re.search(r"wheelbase|Hungarian|SettingsDialog|vehicle_model", t):
            bad(f"G: ControlPanel hosts settings UI: {p.name}")
        else:
            ok(f"G: {p.name} has no settings widgets")

    print("--- H Hungarian ---")
    must_exist("src/domain/scheduling/HungarianAssigner.cpp", "H: HungarianAssigner.cpp")
    must_exist("tests/domain/HungarianAssignerTest.cpp", "H: HungarianAssignerTest")
    must_contain(
        "tests/domain/HungarianAssignerTest.cpp",
        r"Greedy",
        "H: comparison vs Greedy",
    )

    print("--- I docs ---")
    must_exist("docs/decisions/009-bicycle-control-command.md", "I: ADR-009")
    must_exist("docs/decisions/010-ros2-bridge.md", "I: ADR-010")
    must_contain("docs/DEVELOPMENT_PLAN.md", r"\*\*4\*\*.*✅", "I: DEVELOPMENT_PLAN Phase4 done")
    must_contain("docs/MUTATION_CHECKLIST.md", r"M24", "I: MUTATION M24+")
    must_exist("SESSION_LOG.md", "I: SESSION_LOG")

    print("--- J tests registered (Build is user gate) ---")
    tests_cmake = read("tests/CMakeLists.txt")
    for name in (
        "BicycleModelTest.cpp",
        "SteeringAdapterTest.cpp",
        "BicycleScenarioTest.cpp",
        "HungarianAssignerTest.cpp",
    ):
        if name in tests_cmake:
            ok(f"J: tests CMake lists {name}")
        else:
            bad(f"J: tests CMake missing {name}")
    print(
        "NOTE  J runtime: user must Configure+Build+Run FleetSimTests "
        "(Agent does not rebuild on Chinese paths)."
    )

    print("\n========")
    print(f"PASS={len(passes)} FAIL={len(fails)}")
    for f in fails:
        print(f"  - {f}")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
