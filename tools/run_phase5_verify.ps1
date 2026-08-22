# Phase 5 static evidence (no Qt Build). Runtime green is user-local.
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
python tools/verify_phase5_evidence.py
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host ""
Write-Host "Next (user): Qt Configure -> Build FleetSimTests -> run all tests."
Write-Host "Focus: Hybrid* Dubins* Stanley* PlannerSwitch* Priority* MultiAgv* Bicycle*"
