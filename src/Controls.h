#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Camera.h"

#include <glm/vec2.hpp>

// Mouse look and WASD walking. Controls turn the camera; movement is returned
// so the game can pass it through wall collision first.
class Controls {
public:
    Controls(GLFWwindow* window, Camera& camera);

    void setCaptured(bool captured);   // hide the cursor and use the mouse for looking
    bool captured() const { return captured_; }

    // Turns the camera with the mouse and returns the wanted movement on the ground (x, z)
    glm::vec2 update(float deltaSeconds);

private:
    GLFWwindow* window_;
    Camera& camera_;
    bool captured_ = false;
    double lastX_ = 0.0;
    double lastY_ = 0.0;
};
