// The Dark Maze - a first-person escape from a circular maze lit only by a flashlight.
//
//   maze_escape               play a new random maze
//   maze_escape --seed 42     play maze number 42 again
//   maze_escape --frames 120  quit after 120 frames (quick automatic test)
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Config.h"
#include "Controls.h"
#include "Flashlight.h"
#include "Maze.h"
#include "Scene.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <stdexcept>

namespace {

struct Options {
    unsigned seed = 0;
    int frames = 0;   // > 0: quit after this many frames
};

Options parseOptions(int argc, char** argv) {
    Options options;
    options.seed = std::random_device{}();
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--seed") == 0) options.seed = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
        else if (std::strcmp(argv[i], "--frames") == 0) options.frames = std::atoi(argv[++i]);
    }
    return options;
}

GLFWwindow* createWindow() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "The Dark Maze", nullptr, nullptr);
    if (!window) throw std::runtime_error("Could not create an OpenGL 3.3 window (update the graphics driver)");
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);   // vsync
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("GLAD could not load the OpenGL functions");
    return window;
}

// Everything that owns OpenGL objects lives in here, so it is destroyed before the window
void run(GLFWwindow* window, const Options& options, const char* exePath) {
    Maze maze(options.seed);
    Scene scene(findShaderDir(exePath));
    scene.build(maze);
    std::printf("Maze seed %u: %d cells, exit in outer sector %d, %zu wall triangles\n", maze.seed(),
                maze.cellCount(), maze.exitSector(), scene.wallTriangles());
    std::printf("Mouse to look, WASD or arrow keys to walk, Shift to run, Esc to quit\n");

    Camera camera(glm::vec3(0.0f, cfg::kEyeHeight, 0.0f));   // centre of the maze
    Controls controls(window, camera);
    Flashlight flashlight;

    double lastTime = glfwGetTime(), fpsTime = lastTime;
    int fpsFrames = 0, frame = 0;
    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const float dt = std::min(static_cast<float>(now - lastTime), 0.1f);   // no big jumps after a pause
        lastTime = now;

        glfwPollEvents();
        controls.update(dt);
        flashlight.follow(camera);

        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0) {   // minimised: wait instead of spinning
            glfwWaitEvents();
            continue;
        }
        glViewport(0, 0, width, height);
        scene.draw(camera, static_cast<float>(width) / height, flashlight);
        glfwSwapBuffers(window);

        ++fpsFrames;
        if (now - fpsTime >= 1.0) {
            char title[64];
            std::snprintf(title, sizeof(title), "The Dark Maze - %d FPS", fpsFrames);
            glfwSetWindowTitle(window, title);
            fpsFrames = 0;
            fpsTime = now;
        }
        if (options.frames > 0 && ++frame >= options.frames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (const GLenum error = glGetError(); error != GL_NO_ERROR) std::printf("OpenGL error 0x%04X\n", error);
}

}  // namespace

int main(int argc, char** argv) {
    if (!glfwInit()) {
        std::fprintf(stderr, "Could not start GLFW\n");
        return 1;
    }
    int exitCode = 0;
    try {
        GLFWwindow* window = createWindow();
        run(window, parseOptions(argc, argv), argv[0]);
        glfwDestroyWindow(window);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        exitCode = 1;
    }
    glfwTerminate();
    return exitCode;
}
