#include "Controls.h"
#include "Config.h"

Controls::Controls(GLFWwindow* window, Camera& camera) : window_(window), camera_(camera) {}

void Controls::setCaptured(bool captured) {
    captured_ = captured;
    // A disabled cursor is hidden and kept inside the window, so the mouse can turn without limits
    glfwSetInputMode(window_, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, captured ? GLFW_TRUE : GLFW_FALSE);
    glfwGetCursorPos(window_, &lastX_, &lastY_);   // start from here, so the view does not jump
}

glm::vec2 Controls::update(float deltaSeconds) {
    if (!captured_) return glm::vec2(0.0f);
    auto down = [this](int key) { return glfwGetKey(window_, key) == GLFW_PRESS; };

    // Mouse look: moving right turns right, moving up looks up
    double x = 0.0, y = 0.0;
    glfwGetCursorPos(window_, &x, &y);
    camera_.rotate(static_cast<float>(x - lastX_) * cfg::kMouseSensitivity,
                   static_cast<float>(lastY_ - y) * cfg::kMouseSensitivity);
    lastX_ = x;
    lastY_ = y;

    // Walking is relative to where the camera faces, but always stays flat on the ground
    glm::vec3 move(0.0f);
    if (down(GLFW_KEY_W) || down(GLFW_KEY_UP)) move += camera_.flatFront();
    if (down(GLFW_KEY_S) || down(GLFW_KEY_DOWN)) move -= camera_.flatFront();
    if (down(GLFW_KEY_D) || down(GLFW_KEY_RIGHT)) move += camera_.right();
    if (down(GLFW_KEY_A) || down(GLFW_KEY_LEFT)) move -= camera_.right();
    if (glm::length(move) == 0.0f) return glm::vec2(0.0f);

    const bool sprint = down(GLFW_KEY_LEFT_SHIFT) || down(GLFW_KEY_RIGHT_SHIFT);
    const float speed = cfg::kWalkSpeed * (sprint ? cfg::kSprintFactor : 1.0f);
    move = glm::normalize(move) * speed * deltaSeconds;
    return glm::vec2(move.x, move.z);
}
