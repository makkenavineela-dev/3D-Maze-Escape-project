#pragma once
#include "Camera.h"
#include <glm/glm.hpp>

// Spotlight carried by the player. The beam is wide and soft so it fills the whole
// view like natural sight in the dark: bright in the middle, fading towards the screen edges.
struct Flashlight {
    glm::vec3 position{0.0f};
    glm::vec3 direction{1.0f, 0.0f, 0.0f};
    glm::vec3 color{1.0f, 0.93f, 0.8f};   // warm white
    float intensity = 2.0f;
    float innerConeDegrees = 30.0f;         // full brightness inside this angle
    float outerConeDegrees = 60.0f;         // fades to dark between the inner and outer angle
    float constant = 1.0f;                  // distance attenuation: 1 / (c + l*d + q*d^2)
    float linear = 0.09f;
    float quadratic = 0.032f;
    bool on = true;

    // The light sits at the player's eye and points where the player looks
    void follow(const Camera& camera) {
        position = camera.position;
        direction = camera.front();
    }
};
