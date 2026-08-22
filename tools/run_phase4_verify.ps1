#Requires -Version 5.1
<#
.SYNOPSIS
  Phase 4 local verify helper (YOU run this — Agent will not Qt-build on Chinese paths).

.DESCRIPTION
  1) Optional: run static A–J evidence script
  2) Print exact Qt Creator / ctest steps
  3) If FleetSimTests.exe exists AND is newer than Phase4 fix commit time, run filtered tests

.EXAMPLE
  pwsh -File tools/run_phase4_verify.ps1
#>

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

Write-Host "=== FleetSim Phase 4 local verify ===" -ForegroundColor Cyan
Write-Host "ROOT: $Root"

Write-Host "`n[1/3] Static evidence (no Build)..." -ForegroundColor Yellow
python "$Root\tools\verify_phase4_evidence.py"
if ($LASTEXITCODE -ne 0) {
    Write-Host "Static evidence FAILED — fix files before Build." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "`n[2/3] Locate FleetSimTests.exe..." -ForegroundColor Yellow
$testExe = Get-ChildItem -Path "$Root\build" -Recurse -Filter "FleetSimTests.exe" -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $testExe) {
    Write-Host @"

NO FleetSimTests.exe found under build/.

Do this in Qt Creator (required for Goal J):
  1. git pull origin main
  2. Projects → Kit → Re-Configure (MinGW Debug)
  3. Build → Build Project "FleetSim"
  4. Re-run: pwsh -File tools/run_phase4_verify.ps1

Or after Configure, from the build folder:
  cmake --build . --target FleetSimTests
  ctest -R "Bicycle|Steering|Hungarian|MultiAgv" --output-on-failure

"@ -ForegroundColor Yellow
    exit 2
}

Write-Host ("Found: {0}" -f $testExe.FullName)
Write-Host ("LastWrite: {0}" -f $testExe.LastWriteTime)

# Phase4 Open-Project fix pushed ~ 2026-08-23; refuse stale binaries.
$minStamp = Get-Date "2026-08-23 00:30:00"
if ($testExe.LastWriteTime -lt $minStamp) {
    Write-Host @"

STALE binary (before Phase 4 Sessions 3–5 / Open-Project fix).
LastWrite=$($testExe.LastWriteTime) < required >= $minStamp

Please Re-Configure + Rebuild in Qt Creator, then re-run this script.
Do NOT trust ctest results from this exe for Goal completion.

"@ -ForegroundColor Red
    exit 3
}

Write-Host "`n[3/3] Running Phase4-focused gtests..." -ForegroundColor Yellow
$filter = "Bicycle*|Steering*|Hungarian*|VehicleTest*|PurePursuit*|MultiAgv*|DemoScenario*"
& $testExe.FullName --gtest_filter=$filter
$code = $LASTEXITCODE
if ($code -eq 0) {
    Write-Host "`nFleetSimTests filter PASS. Reply in Cursor: Build/Tests 通过" -ForegroundColor Green
} else {
    Write-Host "`nFleetSimTests filter FAILED (exit $code)." -ForegroundColor Red
}
exit $code
