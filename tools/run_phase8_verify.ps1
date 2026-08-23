# Phase 8 static verify + optional ASCII build hint (PowerShell)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root
python "$Root\tools\verify_phase8_evidence.py"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host ""
Write-Host "Static OK. Runtime: configure ASCII out-of-tree e.g. D:\build\FleetSim_phase8 then run FleetSimTests.exe"
