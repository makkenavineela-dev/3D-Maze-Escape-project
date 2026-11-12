cmake_minimum_required(VERSION 3.16)
project(MazeEscape LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MinGW: link the C++ runtime statically so the .exe runs on lab machines without extra DLLs
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

# GLFW - window, OpenGL context, keyboard and mouse (built from source, works with MinGW and MSVC)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLAD - OpenGL 3.3 core function loader
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# GLM (maths) and stb_image (texture loading) are header-only
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)

# Setup check: proves every library compiles, links and can open an OpenGL 3.3 window
add_executable(check_setup tools/check_setup.cpp)
target_link_libraries(check_setup PRIVATE glfw glad glm stb)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(PROJECT_WARNINGS -Wall -Wextra)
elseif(MSVC)
    set(PROJECT_WARNINGS /W4)
endif()

# Maze generation in plain C++ (no OpenGL), so it can be tested on its own
add_library(maze_logic STATIC src/Maze.cpp)
target_include_directories(maze_logic PUBLIC src)
target_link_libraries(maze_logic PUBLIC glm)
target_compile_options(maze_logic PRIVATE ${PROJECT_WARNINGS})

# Rendering: shaders, meshes, camera and the scene
add_library(maze_core STATIC src/Camera.cpp src/Mesh.cpp src/MazeMesh.cpp src/Scene.cpp src/Shader.cpp)
target_link_libraries(maze_core PUBLIC maze_logic glfw glad glm stb)
target_compile_options(maze_core PRIVATE ${PROJECT_WARNINGS})

# Copy the shaders next to the programs on every build
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)

# The game
add_executable(maze_escape src/main.cpp src/Controls.cpp)
target_link_libraries(maze_escape PRIVATE maze_core)
target_compile_options(maze_escape PRIVATE ${PROJECT_WARNINGS})
add_dependencies(maze_escape copy_shaders)

# Test tools: maze checks over many seeds, and still pictures of the rendered maze
add_executable(maze_test tools/maze_test.cpp)
target_link_libraries(maze_test PRIVATE maze_logic)
target_compile_options(maze_test PRIVATE ${PROJECT_WARNINGS})

add_executable(snapshot tools/snapshot.cpp)
target_link_libraries(snapshot PRIVATE maze_core)
target_compile_options(snapshot PRIVATE ${PROJECT_WARNINGS})
add_dependencies(snapshot copy_shaders)
