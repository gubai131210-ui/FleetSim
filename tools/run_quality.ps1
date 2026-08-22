# FleetSim quality harness: build (coverage) -> test -> gcovr -> lizard -> CRAP
param(
    [string]$BuildDir = "build-quality",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

function Require-Command($Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Required command not found: $Name"
    }
}

Require-Command cmake
Require-Command python

if (-not $SkipBuild) {
    cmake -S . -B $BuildDir -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build $BuildDir --config Debug
    ctest --test-dir $BuildDir --output-on-failure
}

$coverageJson = Join-Path $BuildDir "coverage.json"
$lizardCsv = Join-Path $BuildDir "lizard.csv"
$crapReport = Join-Path $BuildDir "crap_report.txt"

Require-Command gcovr
gcovr --root $Root `
    --filter "$Root/src/core" `
    --filter "$Root/src/domain" `
    --json $coverageJson `
    --exclude-unreachable-branches `
    --exclude-throw-branches

Require-Command lizard
lizard -l cpp -C 15 -o $lizardCsv "$Root/src/domain" "$Root/src/core"

python "$Root/tools/crap_score.py" `
    --coverage-json $coverageJson `
    --lizard-csv $lizardCsv `
    --output $crapReport

Write-Host "CRAP report written to $crapReport"
