# One-time setup for 3D Maze Escape.
# Finds (or installs with winget) a 64-bit C++ compiler, CMake and a build tool,
# adds the VS Code extensions, then builds the project and runs the setup check.
param(
    # Optional: force a compiler folder, e.g. "C:\msys64\ucrt64\bin"
    [string]$CompilerDir = ""
)
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot

function Write-Step($text) {
    Write-Host ""
    Write-Host "==> $text" -ForegroundColor Cyan
}

function Update-PathFromSystem {
    $env:PATH = [Environment]::GetEnvironmentVariable("Path", "Machine") + ";" +
                [Environment]::GetEnvironmentVariable("Path", "User")
}

# Folder of a 64-bit g++ that is GCC 9 or newer, or $null
function Find-Compiler {
    $dirs = @()
    if ($CompilerDir) { $dirs += $CompilerDir }
    Get-Command g++ -All -ErrorAction SilentlyContinue | ForEach-Object { $dirs += (Split-Path $_.Source) }
    $dirs += "C:\msys64\ucrt64\bin", "C:\msys64\mingw64\bin",
             "C:\Program Files\CodeBlocks\MinGW\bin", "C:\Program Files (x86)\CodeBlocks\MinGW\bin"
    $dirs += Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\BrechtSanders.WinLibs*\mingw64\bin" `
                 -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName }
    foreach ($d in $dirs) {
        $gpp = Join-Path $d "g++.exe"
        if (-not (Test-Path $gpp)) { continue }
        $machine = & $gpp -dumpmachine
        $major = [int](((& $gpp -dumpversion) -split "\.")[0])
        if ($machine -like "x86_64*" -and $major -ge 9) { return $d }
        Write-Host "  skipping $gpp (GCC $major, $machine - too old or 32-bit)"
    }
    return $null
}

# Full path of a tool, looking in the given folders first and then on PATH
function Find-Tool($name, $dirs) {
    foreach ($d in $dirs) {
        $p = Join-Path $d $name
        if (Test-Path $p) { return $p }
    }
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    return $null
}

function Install-WithWinget($id) {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        throw "winget is missing. Install 'App Installer' from the Microsoft Store, then run setup again."
    }
    Write-Host "Installing $id with winget (this can take a few minutes)..."
    winget install --id $id -e --source winget
    Update-PathFromSystem
}

try {
    Write-Step "Looking for a 64-bit C++ compiler (GCC 9 or newer)"
    $bin = Find-Compiler
    if (-not $bin) {
        # WinLibs = GCC + CMake + Ninja in one package
        Install-WithWinget "BrechtSanders.WinLibs.POSIX.UCRT"
        $bin = Find-Compiler
        if (-not $bin) { throw "g++ still not found. Close this window, open a new one and run setup again." }
    }
    Write-Host "Compiler : $bin"

    Write-Step "Looking for CMake"
    $toolDirs = @($bin, "C:\Program Files\CMake\bin", "$env:LOCALAPPDATA\Microsoft\WinGet\Links")
    $cmake = Find-Tool "cmake.exe" $toolDirs
    if (-not $cmake) {
        Install-WithWinget "Kitware.CMake"
        $cmake = Find-Tool "cmake.exe" $toolDirs
        if (-not $cmake) { throw "CMake still not found. Close this window, open a new one and run setup again." }
    }
    Write-Host "CMake    : $cmake"

    Write-Step "Looking for a build tool"
    $ninja = Find-Tool "ninja.exe" $toolDirs
    $make = Join-Path $bin "mingw32-make.exe"
    if ($ninja) {
        $generator = "Ninja"; $makeProgram = $ninja
    } elseif (Test-Path $make) {
        $generator = "MinGW Makefiles"; $makeProgram = $make
    } else {
        Install-WithWinget "Ninja-build.Ninja"
        $ninja = Find-Tool "ninja.exe" $toolDirs
        if (-not $ninja) { throw "Ninja still not found. Close this window, open a new one and run setup again." }
        $generator = "Ninja"; $makeProgram = $ninja
    }
    Write-Host "Build    : $generator ($makeProgram)"

    Write-Step "Adding VS Code extensions"
    if (Get-Command code -ErrorAction SilentlyContinue) {
        foreach ($ext in "ms-vscode.cpptools", "ms-vscode.cmake-tools", "slevesque.shader") {
            code --install-extension $ext | Out-Null
            Write-Host "  $ext"
        }
    } else {
        Write-Host "VS Code not found - skipped. Install it from https://code.visualstudio.com and run setup again." `
            -ForegroundColor Yellow
    }

    Write-Step "Building the project"
    $env:PATH = "$bin;$env:PATH"
    $build = Join-Path $root "build"
    # Start from a clean configure so a build folder copied from another laptop can't break it
    Remove-Item -Recurse -Force (Join-Path $build "CMakeCache.txt"), (Join-Path $build "CMakeFiles") `
        -ErrorAction SilentlyContinue
    $fwd = { param($p) $p -replace "\\", "/" }
    & $cmake -S $root -B $build -G $generator -DCMAKE_BUILD_TYPE=Release `
        "-DCMAKE_MAKE_PROGRAM=$(& $fwd $makeProgram)" `
        "-DCMAKE_C_COMPILER=$(& $fwd (Join-Path $bin 'gcc.exe'))" `
        "-DCMAKE_CXX_COMPILER=$(& $fwd (Join-Path $bin 'g++.exe'))"
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed (see the messages above)." }
    & $cmake --build $build
    if ($LASTEXITCODE -ne 0) { throw "Build failed (see the messages above)." }

    Write-Step "Running the setup check"
    & (Join-Path $build "check_setup.exe")
    if ($LASTEXITCODE -ne 0) { throw "The setup check failed. If it mentions OpenGL 3.3, update the graphics driver." }

    Write-Host ""
    Write-Host "Setup complete. Play the game with build\maze_escape.exe" -ForegroundColor Green
    Write-Host "To build again later:  powershell -ExecutionPolicy Bypass -File setup.ps1   (or double-click setup.bat)"
    exit 0
} catch {
    Write-Host ""
    Write-Host "SETUP FAILED: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
