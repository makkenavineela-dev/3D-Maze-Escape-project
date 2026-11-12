#include "Camera.h"
#include "Config.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
const glm::vec3 kWorldUp(0.0f, 1.0f, 0.0f);
constexpr float kMaxPitch = 89.0f;   // looking straight up or down would break lookAt (gimbal lock)
}

Camera::Camera(glm::vec3 position, float yawDegrees, float pitchDegrees)
    : position(position), fovDegrees(cfg::kFovDegrees), yaw_(yawDegrees),
      pitch_(std::clamp(pitchDegrees, -kMaxPitch, kMaxPitch)) {}

glm::mat4 Camera::view() const { return glm::lookAt(position, position + front(), kWorldUp); }

glm::mat4 Camera::projection(float aspect) const {
    return glm::perspective(glm::radians(fovDegrees), aspect, nearPlane, farPlane);
}

glm::vec3 Camera::front() const {
    const float yaw = glm::radians(yaw_), pitch = glm::radians(pitch_);
    return glm::normalize(glm::vec3(std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch)));
}

glm::vec3 Camera::flatFront() const {
    const float yaw = glm::radians(yaw_);
    return glm::vec3(std::cos(yaw), 0.0f, std::sin(yaw));
}

glm::vec3 Camera::right() const { return glm::normalize(glm::cross(flatFront(), kWorldUp)); }

void Camera::rotate(float yawDegrees, float pitchDegrees) {
    yaw_ = std::fmod(yaw_ + yawDegrees, 360.0f);
    pitch_ = std::clamp(pitch_ + pitchDegrees, -kMaxPitch, kMaxPitch);
}
