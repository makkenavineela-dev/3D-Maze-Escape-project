#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Camera.h"

// Mouse look and WASD walking (Person 1). Movement has no collision yet; that comes in Stage 3.
class Controls {
public:
    Controls(GLFWwindow* window, Camera& camera);
    void update(float deltaSeconds);

private:
    GLFWwindow* window_;
    Camera& camera_;
    double lastX_ = 0.0;
    double lastY_ = 0.0;
};
