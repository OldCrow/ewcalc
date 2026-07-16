# scripts/find-msbuild.ps1
# Locates MSBuild.exe on this machine and prints its full path to stdout.
# Shared by scripts/build-windows.ps1 and CMakeLists.txt's EWCALC_BUILD_FRONTEND
# target so both use one lookup implementation instead of two that could drift.
#
# Resolution order:
#   1. vswhere — understands VS "components" and reports the newest complete
#      installation cleanly. This is the preferred, officially-supported path.
#   2. Filesystem scan of the two standard VS install roots, picking the
#      highest-versioned MSBuild.exe found. This fallback covers two cases:
#        - vswhere's cached instance-state reader lagging the actual
#          installer version after an in-place VS upgrade, silently
#          reporting no installations even though VS is fully usable.
#        - vswhere being absent entirely (e.g. a minimal Build Tools layout
#          extracted without the full Installer suite).
#      Neither install root nor "MSBuild\Current\Bin" is version- or
#      edition-specific, so this covers any VS version/edition (Community/
#      Professional/Enterprise/BuildTools, 2017 through any future release) —
#      not just the specific version installed on any one machine.
#
# On success, prints the MSBuild.exe path and exits 0.
# On failure, prints nothing and exits 1.

$ErrorActionPreference = "Stop"

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsInstallPath = & $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -property installationPath
    if ($vsInstallPath) {
        $candidate = Join-Path $vsInstallPath "MSBuild\Current\Bin\MSBuild.exe"
        if (Test-Path $candidate) {
            Write-Output $candidate
            exit 0
        }
    }
}

$roots = @(
    "C:\Program Files\Microsoft Visual Studio",
    "C:\Program Files (x86)\Microsoft Visual Studio"
)
$candidates = foreach ($root in $roots) {
    if (Test-Path $root) {
        Get-ChildItem -Path $root -Recurse -Filter "MSBuild.exe" -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -like "*\MSBuild\Current\Bin\MSBuild.exe" }
    }
}
$best = $candidates |
    Sort-Object { [version]($_.VersionInfo.FileVersion -replace '[^\d.]', '') } -Descending |
    Select-Object -First 1
if ($best) {
    Write-Output $best.FullName
    exit 0
}

exit 1
