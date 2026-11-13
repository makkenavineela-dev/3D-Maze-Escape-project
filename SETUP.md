# 3D Maze Escape - Setup

All libraries are already inside `external/`, so nothing else has to be downloaded.

| Folder | Library | Version | Used for |
|---|---|---|---|
| `external/glfw` | GLFW (source, built by CMake) | 3.4 | Window, OpenGL context, keyboard and mouse |
| `external/glad` | GLAD (generated) | OpenGL 3.3 core | Loads OpenGL functions |
| `external/glm` | GLM (headers) | 1.0.1 | Vectors, matrices, lookAt, perspective |
| `external/stb` | stb_image (single header) | 2.30 | Loading wall and ground textures |

## Quick setup (do this once on every laptop)

1. Unzip the project folder anywhere (for example the Desktop).
2. Double-click **`setup.bat`**. If Windows shows "Windows protected your PC", click More info → Run anyway.
3. Wait until it prints `ALL OK` and `Setup complete`.

The script finds a suitable compiler, CMake and Ninja on your laptop. If they are missing, it installs them with winget; answer **Y** if winget asks you to accept its terms.
It then adds the three VS Code extensions, builds the project and runs the setup check.
If it picks the wrong compiler, run `powershell -ExecutionPolicy Bypass -File setup.ps1 -CompilerDir "C:\path\to\mingw64\bin"`.

## Tools you need

- A 64-bit C++ compiler: MSYS2 UCRT64 GCC, the MinGW-w64 that comes with Code::Blocks, or Visual Studio 2022 (C++ workload)
- CMake 3.16 or newer (and Ninja if you use MinGW)
- VS Code extensions: C/C++ (`ms-vscode.cpptools`), CMake Tools (`ms-vscode.cmake-tools`), Shader languages support (`slevesque.shader`) for `.vert` / `.frag` syntax highlighting

Do not use the old 32-bit `C:\MinGW` (GCC 6.3). It is too old for this project.

## Build and test

MinGW / MSYS2 (PowerShell):

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\build\check_setup.exe
```

Visual Studio 2022 (needs the "Desktop development with C++" workload, which includes the Windows SDK):
File → Open → Folder, choose this project folder, wait for CMake to finish, pick `check_setup.exe` as the startup item and run it.

`check_setup` should print the OpenGL version (3.3 or higher) and end with `ALL OK`.
If it says it could not create an OpenGL 3.3 window, update the graphics driver.

## Play (Stages 1 to 4 are done)

Run `build\maze_escape.exe`. Find the single exit in the outer wall and walk a few steps outside to win.

| Key | Action |
|---|---|
| Click / Enter | Start playing; on the victory screen, a new maze |
| Mouse | Look around |
| W A S D or arrow keys | Walk (Shift to run) |
| F | Flashlight on/off |
| 1 - 6 | Lighting demo: 1 full, 2 no cone, 3 no distance attenuation, 4 no fog, 5 ambient only, 6 daylight (whole maze lit) |
| R | Give up and start a new maze |
| F3 | Show FPS, maze number and time |
| F11 | Full screen on/off |
| Esc | Quit |

Options: `--seed 42` starts with maze 42, `--autoplay` lets the game walk itself out of every maze (demo),
`--frames 600` quits after 600 frames and prints the average FPS, `--novsync` shows how fast the PC really is.

## Test tools

| Program | What it does |
|---|---|
| `build\maze_test.exe` | Checks 1000 random mazes (every cell reachable, no loops, exactly one exit), then collision: 200 random walks of 20000 steps must never get into or through a wall, and an autopilot must walk out of every maze. `maze_test --svg maze.svg 42` draws maze 42 from above; open the .svg in a browser. |
| `build\snapshot.exe` | Saves pictures without playing into a `snapshots` folder: `top`, `start`, `corridor`, `exit` and the screens `ui_start`, `ui_playing`, `ui_victory` (.bmp). `--seed 42` picks the maze. |
| `python tools/make_textures.py` | Regenerates `textures/` (brick wall, ground, font). Needs Python with numpy, Pillow and matplotlib. |

## Who owns which file

| Person | Files |
|---|---|
| Everyone | `src/Config.h` (shared sizes agreed at the kickoff) |
| Person 1: engine, camera, game flow | `src/main.cpp`, `src/Game.*`, `src/Camera.*`, `src/Controls.*` |
| Person 2: maze and collision | `src/Maze.*` (`WallSegment`, `worldToCell`, `exitSector`), `src/Collision.*`, `src/Autopilot.*`, `tools/maze_test.cpp` |
| Person 3: geometry, textures, UI | `src/Mesh.*`, `src/MazeMesh.*`, `src/Texture.*`, `src/Overlay.*`, `src/Hud.*`, `shaders/overlay.*`, `textures/`, `tools/make_textures.py`, `src/Scene.*` (shared with Person 4) |
| Person 4: lighting and shaders | `src/Shader.*`, `src/Flashlight.h`, `src/LightingModes.h`, `src/Exit.h`, `shaders/scene.*`, `shaders/sky.*` |

Shaders and textures are copied next to the programs on every build, so after editing one just build again.
