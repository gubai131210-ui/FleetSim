#!/usr/bin/env python3
"""Compute CRAP scores from gcovr JSON + Lizard complexity CSV."""

from __future__ import annotations

import argparse
import csv
import json
import math
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple


def crap_score(cc: int, coverage: float) -> float:
    cov = max(0.0, min(1.0, coverage))
    return float(cc * cc * math.pow(1.0 - cov, 3) + cc)


@dataclass
class FunctionMetrics:
    file_path: str
    function_name: str
    complexity: int
    coverage: float
    crap: float


def normalize_path(path: str) -> str:
    return path.replace("\\", "/").lower()


def load_lizard_rows(lizard_csv: Path) -> List[Tuple[str, str, int]]:
    rows: List[Tuple[str, str, int]] = []
    with lizard_csv.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            file_path = row.get("filename") or row.get("file") or ""
            function_name = row.get("name") or row.get("function") or ""
            cc_text = row.get("ccn") or row.get("complexity") or "1"
            try:
                complexity = int(float(cc_text))
            except ValueError:
                complexity = 1
            if file_path and function_name:
                rows.append((file_path, function_name, max(1, complexity)))
    return rows


def load_gcov_functions(coverage_json: Path) -> Dict[Tuple[str, str], float]:
    with coverage_json.open(encoding="utf-8") as handle:
        payload = json.load(handle)

    coverage_map: Dict[Tuple[str, str], float] = {}
    files = payload.get("files", [])
    for file_entry in files:
        file_path = file_entry.get("file", "")
        for fn in file_entry.get("functions", []):
            name = fn.get("name", "")
            if not name:
                continue
            lines = fn.get("lines", {})
            if not lines:
                coverage_map[(normalize_path(file_path), name)] = 0.0
                continue
            covered = sum(1 for hit in lines.values() if hit > 0)
            total = len(lines)
            ratio = covered / total if total else 0.0
            coverage_map[(normalize_path(file_path), name)] = ratio
    return coverage_map


def match_coverage(
    file_path: str,
    function_name: str,
    coverage_map: Dict[Tuple[str, str], float],
) -> float:
    norm_file = normalize_path(file_path)
    exact = coverage_map.get((norm_file, function_name))
    if exact is not None:
        return exact

    suffix_matches = [
        value
        for (cov_file, cov_fn), value in coverage_map.items()
        if cov_file.endswith(norm_file.split("/")[-1]) and cov_fn == function_name
    ]
    if suffix_matches:
        return sum(suffix_matches) / len(suffix_matches)

    return 0.0


def build_report(
    lizard_rows: Iterable[Tuple[str, str, int]],
    coverage_map: Dict[Tuple[str, str], float],
    domain_only: bool,
) -> List[FunctionMetrics]:
    metrics: List[FunctionMetrics] = []
    for file_path, function_name, complexity in lizard_rows:
        norm = normalize_path(file_path)
        if domain_only and "/src/domain/" not in norm and not norm.startswith("src/domain/"):
            continue
        coverage = match_coverage(file_path, function_name, coverage_map)
        score = crap_score(complexity, coverage)
        metrics.append(
            FunctionMetrics(
                file_path=file_path,
                function_name=function_name,
                complexity=complexity,
                coverage=coverage,
                crap=score,
            )
        )
    metrics.sort(key=lambda item: item.crap, reverse=True)
    return metrics


def write_report(report_path: Path, metrics: List[FunctionMetrics], warn_threshold: float, fail_threshold: float) -> int:
    high_risk = [m for m in metrics if m.crap > fail_threshold]
    warn = [m for m in metrics if warn_threshold < m.crap <= fail_threshold]

    lines = [
        "FleetSim CRAP Report",
        "CRAP(m) = CC^2 * (1-cov)^3 + CC",
        "",
        f"{'CRAP':>8}  {'CC':>4}  {'Cov%':>6}  Function @ File",
        "-" * 80,
    ]
    for item in metrics:
        flag = ""
        if item.crap > fail_threshold:
            flag = " [FAIL]"
        elif item.crap > warn_threshold:
            flag = " [WARN]"
        lines.append(
            f"{item.crap:8.1f}  {item.complexity:4d}  {item.coverage * 100:6.1f}  "
            f"{item.function_name} @ {item.file_path}{flag}"
        )

    lines.extend(
        [
            "",
            f"Functions with CRAP > {warn_threshold:.0f}: {len(warn) + len(high_risk)}",
            f"Functions with CRAP > {fail_threshold:.0f}: {len(high_risk)}",
        ]
    )

    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines))
    return 1 if high_risk else 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Compute CRAP scores for FleetSim")
    parser.add_argument("--coverage-json", type=Path, required=True)
    parser.add_argument("--lizard-csv", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("build/crap_report.txt"))
    parser.add_argument("--domain-only", action="store_true", default=True)
    parser.add_argument("--warn-threshold", type=float, default=30.0)
    parser.add_argument("--fail-threshold", type=float, default=60.0)
    args = parser.parse_args()

    if not args.coverage_json.exists():
        print(f"Missing coverage JSON: {args.coverage_json}", file=sys.stderr)
        return 2
    if not args.lizard_csv.exists():
        print(f"Missing Lizard CSV: {args.lizard_csv}", file=sys.stderr)
        return 2

    lizard_rows = load_lizard_rows(args.lizard_csv)
    coverage_map = load_gcov_functions(args.coverage_json)
    metrics = build_report(lizard_rows, coverage_map, args.domain_only)
    return write_report(args.output, metrics, args.warn_threshold, args.fail_threshold)


if __name__ == "__main__":
    raise SystemExit(main())
