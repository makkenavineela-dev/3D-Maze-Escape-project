// Renders still pictures of a maze without playing, for checking geometry and lighting.
//
//   snapshot [--seed 42] [--out folder]
//
// Writes four .bmp pictures: top (whole maze from above, fully lit), start (player view at the
// start), corridor (player view half-way along the solution) and exit (looking out of the exit).
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Config.h"
#include "Flashlight.h"
#include "Maze.h"
#include "Scene.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int kWidth = 1280, kHeight = 720;

// 24-bit BMP; rows are stored bottom-up, which is also how glReadPixels returns them
void writeBmp(const std::string& path, const std::vector<unsigned char>& bgr) {
    const int rowSize = (kWidth * 3 + 3) & ~3;
    const std::uint32_t dataSize = static_cast<std::uint32_t>(rowSize * kHeight);
    unsigned char header[54] = {'B', 'M'};
    auto put = [&](int offset, std::uint32_t value, int bytes) {
        for (int i = 0; i < bytes; ++i) header[offset + i] = static_cast<unsigned char>(value >> (8 * i));
    };
    put(2, 54 + dataSize, 4);   // file size
    put(10, 54, 4);             // pixel data offset
    put(14, 40, 4);             // info header size
    put(18, kWidth, 4);
    put(22, kHeight, 4);
    put(26, 1, 2);              // planes
    put(28, 24, 2);             // bits per pixel
    put(34, dataSize, 4);
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(header), sizeof(header));
    file.write(reinterpret_cast<const char*>(bgr.data()), dataSize);
    if (!file) throw std::runtime_error("Cannot write " + path);
}

// Off-screen framebuffer, so nothing has to appear on the screen
struct Framebuffer {
    GLuint fbo = 0, color = 0, depth = 0;
    Framebuffer() {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenRenderbuffers(1, &color);
        glBindRenderbuffer(GL_RENDERBUFFER, color);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kWidth, kHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Off-screen framebuffer is not complete");
    }
    ~Framebuffer() {
        glDeleteRenderbuffers(1, &depth);
        glDeleteRenderbuffers(1, &color);
        glDeleteFramebuffers(1, &fbo);
    }
};

void shoot(const Scene& scene, const Camera& camera, const Flashlight& light, const std::string& path) {
    glViewport(0, 0, kWidth, kHeight);
    scene.draw(camera, static_cast<float>(kWidth) / kHeight, light);
    std::vector<unsigned char> pixels(static_cast<size_t>((kWidth * 3 + 3) & ~3) * kHeight);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, kWidth, kHeight, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());
    writeBmp(path, pixels);
    std::printf("Wrote %s\n", path.c_str());
}

// Player camera standing in cell `from` looking towards the centre of cell `to`
Camera playerView(const Maze& maze, CellId from, CellId to) {
    const glm::vec2 a = maze.cellCentre(from), b = maze.cellCentre(to);
    const float yaw = glm::degrees(std::atan2(b.y - a.y, b.x - a.x));
    return Camera(glm::vec3(a.x, cfg::kEyeHeight, a.y), yaw, 0.0f);
}

void run(unsigned seed, const std::string& outDir, const char* exePath) {
    Maze maze(seed);
    Scene scene(findShaderDir(exePath));
    scene.build(maze);
    Framebuffer framebuffer;
    std::filesystem::create_directories(outDir);
    const std::vector<CellId> path = maze.solve();

    // Whole maze from high above, no flashlight, everything evenly lit
    Flashlight light;
    light.on = false;
    const float darkAmbient = scene.ambient, fogDensity = scene.fogDensity;
    scene.ambient = 1.0f;
    scene.fogDensity = 0.0f;
    const float height = maze.outerRadius() / std::tan(glm::radians(30.0f)) + cfg::kWallHeight + 2.0f;
    Camera top(glm::vec3(0.0f, height, 0.0f), 90.0f, -89.0f);
    top.fovDegrees = 60.0f;
    shoot(scene, top, light, outDir + "/top.bmp");

    // Player views, lit only by the flashlight
    scene.ambient = darkAmbient;
    scene.fogDensity = fogDensity;
    light.on = true;
    Camera start = playerView(maze, path[0], path[1]);
    light.follow(start);
    shoot(scene, start, light, outDir + "/start.bmp");

    const size_t middle = path.size() / 2;
    Camera corridor = playerView(maze, path[middle], path[middle + 1]);
    light.follow(corridor);
    shoot(scene, corridor, light, outDir + "/corridor.bmp");

    // Standing in the exit cell, looking straight out through the gap in the outer wall
    const CellId exitCell = path.back();
    const glm::vec2 c = maze.cellCentre(exitCell);
    const float exitYaw = glm::degrees(std::atan2(c.y, c.x));
    Camera exitView(glm::vec3(c.x, cfg::kEyeHeight, c.y), exitYaw, 0.0f);
    light.follow(exitView);
    shoot(scene, exitView, light, outDir + "/exit.bmp");
}

}  // namespace

int main(int argc, char** argv) {
    unsigned seed = 1;
    std::string outDir = "snapshots";
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--seed") == 0) seed = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
        else if (std::strcmp(argv[i], "--out") == 0) outDir = argv[++i];
    }

    if (!glfwInit()) return 1;
    int exitCode = 0;
    try {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        GLFWwindow* window = glfwCreateWindow(64, 64, "snapshot", nullptr, nullptr);
        if (!window) throw std::runtime_error("Could not create an OpenGL 3.3 context");
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
            throw std::runtime_error("GLAD could not load OpenGL");
        run(seed, outDir, argv[0]);
        if (const GLenum error = glGetError(); error != GL_NO_ERROR) std::printf("OpenGL error 0x%04X\n", error);
        glfwDestroyWindow(window);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        exitCode = 1;
    }
    glfwTerminate();
    return exitCode;
}
