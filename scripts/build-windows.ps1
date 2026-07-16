# scripts/build-windows.ps1
# Builds the full Windows ewcalc stack:
#   1. CMake: libew + ewpresenter (native C++ static libs)
#   2. MSBuild: WinUI 3 frontend (ewcalc-winui.sln)
#   3. Optionally packages as MSIX (pass -Package)
#
# Usage:
#   .\scripts\build-windows.ps1                   # build only
#   .\scripts\build-windows.ps1 -Config Debug     # debug build
#   .\scripts\build-windows.ps1 -Package          # build + create MSIX
#   .\scripts\build-windows.ps1 -Package -Sign    # build + sign + create MSIX
#
# Prerequisites:
#   - Visual Studio 2022/2026 with "Desktop development with C++" and ".NET desktop development"
#   - Windows App SDK 2.x runtime installed
#   - CMake 3.20+ on PATH
#   - (For signing) a code-signing certificate

[CmdletBinding()]
param(
    [string]$Config   = "Release",
    [string]$Platform = "x64",
    [switch]$Package,
    [switch]$Sign,
    [string]$CertThumbprint = ""
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path $PSScriptRoot -Parent
$BuildDir = Join-Path $RepoRoot "build"
$SolutionDir = Join-Path $RepoRoot "frontend\windows\ewcalc-winui"
$Solution = Join-Path $SolutionDir "ewcalc-winui.sln"

# ── 1. Locate MSBuild ────────────────────────────────────────────────────────
Write-Host "`n==> Locating MSBuild..." -ForegroundColor Cyan

# See find-msbuild.ps1 for the vswhere + filesystem-scan fallback rationale;
# CMakeLists.txt's EWCALC_BUILD_FRONTEND target shares the same helper.
$MSBuild = & (Join-Path $PSScriptRoot "find-msbuild.ps1")
if ($LASTEXITCODE -ne 0 -or -not $MSBuild) {
    throw "MSBuild.exe not found via vswhere or filesystem scan — is Visual Studio installed with the C++ and .NET desktop workloads?"
}
Write-Host "    MSBuild: $MSBuild" -ForegroundColor Gray

# Reused below for makeappx/signtool lookup during MSIX packaging. Guarded at
# each call site since it may be missing or stale (see find-msbuild.ps1) —
# both call sites already fall back to well-known Windows SDK paths.
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

# ── 2. CMake: build libew + ewpresenter ─────────────────────────────────────
Write-Host "`n==> Building native libraries (CMake)..." -ForegroundColor Cyan

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Push-Location $RepoRoot
try {
    cmake -B build -DEWCALC_BUILD_TESTS=OFF
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed (exit $LASTEXITCODE)" }

    cmake --build build --config $Config --parallel
    if ($LASTEXITCODE -ne 0) { throw "CMake build failed (exit $LASTEXITCODE)" }
} finally {
    Pop-Location
}

Write-Host "    Native libs built to: $BuildDir\lib\$Config\" -ForegroundColor Gray

# ── 3. MSBuild: WinUI 3 frontend ─────────────────────────────────────────────
Write-Host "`n==> Building WinUI 3 frontend (MSBuild)..." -ForegroundColor Cyan

$MSBuildArgs = @(
    $Solution,
    "/restore",             # restore NuGet packages (C# project + C++/CLI)
    "/p:Configuration=$Config",
    "/p:Platform=$Platform",
    "/m",                   # parallel build
    "/nologo",
    "/verbosity:minimal"
)

& $MSBuild @MSBuildArgs
if ($LASTEXITCODE -ne 0) { throw "MSBuild failed (exit $LASTEXITCODE)" }

$FrontendBin = Join-Path $SolutionDir "ewcalc-winui\bin\$Platform\$Config\net8.0-windows10.0.26100.0"
Write-Host "    Frontend built to: $FrontendBin" -ForegroundColor Gray

# ── 4. Package as MSIX ───────────────────────────────────────────────────────
if ($Package) {
    Write-Host "`n==> Packaging as MSIX..." -ForegroundColor Cyan

    $MsixDir = Join-Path $BuildDir "msix"
    if (-not (Test-Path $MsixDir)) {
        New-Item -ItemType Directory -Path $MsixDir | Out-Null
    }

    # MakeAppx is part of the Windows SDK
    $MakeAppx = $null
    if (Test-Path $VsWhere) {
        $MakeAppx = & $VsWhere -latest -prerelease -find "**\makeappx.exe" | Select-Object -First 1
    }
    if (-not $MakeAppx) {
        # Fall back to well-known Windows SDK path
        $SdkBin = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\makeappx.exe"
        if (Test-Path $SdkBin) { $MakeAppx = $SdkBin }
        else { throw "makeappx.exe not found — install Windows SDK" }
    }

    $MsixOutput = Join-Path $MsixDir "ewcalc-$Platform-$Config.msix"

    # Single-project MSIX (EnableMsixTooling) stages the resolved AppxManifest.xml
    # and payload under an "AppX" subfolder of the build output on Build (not just
    # Publish); pack from there when present, falling back to the bin root for
    # older/alternate layouts.
    $PackSource = Join-Path $FrontendBin "AppX"
    if (-not (Test-Path (Join-Path $PackSource "AppxManifest.xml"))) {
        $PackSource = $FrontendBin
    }
    Write-Host "    Packaging from: $PackSource" -ForegroundColor Gray

    & $MakeAppx pack /d $PackSource /p $MsixOutput /nv /o
    if ($LASTEXITCODE -ne 0) { throw "makeappx failed (exit $LASTEXITCODE)" }

    Write-Host "    MSIX: $MsixOutput" -ForegroundColor Green

    # ── 5. Sign (optional) ───────────────────────────────────────────────────
    if ($Sign) {
        Write-Host "`n==> Signing MSIX..." -ForegroundColor Cyan

        if (-not $CertThumbprint) {
            throw "-Sign requires -CertThumbprint <thumbprint>"
        }

        $SignTool = $null
        if (Test-Path $VsWhere) {
            $SignTool = & $VsWhere -latest -prerelease -find "**\signtool.exe" | Select-Object -First 1
        }
        if (-not $SignTool) {
            $SignTool = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"
            if (-not (Test-Path $SignTool)) { throw "signtool.exe not found" }
        }

        & $SignTool sign /sha1 $CertThumbprint /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 $MsixOutput
        if ($LASTEXITCODE -ne 0) { throw "signtool failed (exit $LASTEXITCODE)" }

        Write-Host "    Signed: $MsixOutput" -ForegroundColor Green
    }
}

Write-Host "`n==> Build complete." -ForegroundColor Green
