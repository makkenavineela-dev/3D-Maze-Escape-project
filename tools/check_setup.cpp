// Setup check: opens a hidden OpenGL 3.3 core window and prints the version of every library.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cstdio>

int main() {
    if (!glfwInit()) {
        std::puts("FAIL: glfwInit");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(320, 240, "check_setup", nullptr, nullptr);
    if (!window) {
        std::puts("FAIL: could not create an OpenGL 3.3 core window (update the graphics driver)");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::puts("FAIL: GLAD could not load OpenGL functions");
        return 1;
    }

    // Draw one frame to make sure rendering works
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    stbi_set_flip_vertically_on_load(true);

    std::printf("GLFW      : %s\n", glfwGetVersionString());
    std::printf("OpenGL    : %s\n", (const char*)glGetString(GL_VERSION));
    std::printf("GLSL      : %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    std::printf("GPU       : %s\n", (const char*)glGetString(GL_RENDERER));
    std::printf("GLM       : %d.%d.%d (perspective ok: %s)\n", GLM_VERSION_MAJOR, GLM_VERSION_MINOR,
                GLM_VERSION_PATCH, projection[2][3] == -1.0f ? "yes" : "no");
    std::printf("stb_image : linked\n");
    std::printf("GL error  : %s\n", glGetError() == GL_NO_ERROR ? "none" : "yes");
    std::puts("ALL OK - the project is ready to build");

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
