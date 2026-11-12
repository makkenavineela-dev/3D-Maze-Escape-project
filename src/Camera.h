#pragma once
#include <glm/glm.hpp>

// First-person camera (Person 1): yaw/pitch angles, lookAt view matrix and perspective projection.
// Yaw 0 looks along +X; yaw grows towards +Z (turning right).
class Camera {
public:
    explicit Camera(glm::vec3 position, float yawDegrees = 0.0f, float pitchDegrees = 0.0f);

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    glm::vec3 front() const;       // where the camera looks
    glm::vec3 flatFront() const;   // front flattened onto the ground, for walking
    glm::vec3 right() const;

    void rotate(float yawDegrees, float pitchDegrees);   // pitch is clamped to +-89 degrees
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }

    glm::vec3 position;
    float fovDegrees;
    float nearPlane = 0.05f;
    float farPlane = 200.0f;

private:
    float yaw_;
    float pitch_;
};
