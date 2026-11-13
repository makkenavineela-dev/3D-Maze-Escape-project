// The Dark Maze - a first-person escape from a circular maze lit only by a flashlight.
//
//   maze_escape               play, starting with a new random maze
//   maze_escape --seed 42     start with maze number 42
//   maze_escape --autoplay    demo: the game walks itself out of each maze
//   maze_escape --frames 120  quit after 120 frames and print the average FPS (quick test)
//   maze_escape --novsync     do not wait for the screen refresh (shows how fast the PC really is)
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Game.h"
#include "Scene.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <stdexcept>

namespace {

struct Options {
    unsigned seed = 0;
    int frames = 0;   // > 0: quit after this many frames
    bool autoplay = false;
    bool vsync = true;
};

Options parseOptions(int argc, char** argv) {
    Options options;
    options.seed = std::random_device{}();
    for (int i = 1; i < argc; ++i) {
        const bool hasValue = i + 1 < argc;
        if (std::strcmp(argv[i], "--seed") == 0 && hasValue) options.seed = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
        else if (std::strcmp(argv[i], "--frames") == 0 && hasValue) options.frames = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--autoplay") == 0) options.autoplay = true;
        else if (std::strcmp(argv[i], "--novsync") == 0) options.vsync = false;
    }
    return options;
}

GLFWwindow* createWindow(bool vsync) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "The Dark Maze", nullptr, nullptr);
    if (!window) throw std::runtime_error("Could not create an OpenGL 3.3 window (update the graphics driver)");
    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync ? 1 : 0);   // 1 = wait for the screen refresh (no tearing)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("GLAD could not load the OpenGL functions");
    return window;
}

}  // namespace

int main(int argc, char** argv) {
    if (!glfwInit()) {
        std::fprintf(stderr, "Could not start GLFW\n");
        return 1;
    }
    int exitCode = 0;
    try {
        const Options options = parseOptions(argc, argv);
        GLFWwindow* window = createWindow(options.vsync);
        {
            Game game(window, findAssetRoot(argv[0]), options.seed, options.autoplay);   // owns all OpenGL objects
            game.run(options.frames);
        }
        glfwDestroyWindow(window);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        exitCode = 1;
    }
    glfwTerminate();
    return exitCode;
}
