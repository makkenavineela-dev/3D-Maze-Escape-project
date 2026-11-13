#include "Autopilot.h"
#include "Config.h"

#include <algorithm>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

namespace {
constexpr float kReached = 0.4f;   // close enough to a route point to head for the next one
}

Autopilot::Autopilot(const Maze& maze) {
    const std::vector<CellId> path = maze.solve();
    for (size_t i = 1; i < path.size(); ++i) route_.push_back(maze.cellCentre(path[i]));
    // finally walk straight out through the exit gap
    const float exitAngle = (maze.exitSector() + 0.5f) * glm::two_pi<float>() / maze.sectors(maze.rings() - 1);
    const float outside = maze.outerRadius() + cfg::kExitDistance + 2.0f;
    route_.push_back(glm::vec2(std::cos(exitAngle), std::sin(exitAngle)) * outside);
}

glm::vec2 Autopilot::step(glm::vec2 position, float distance) {
    while (next_ < route_.size() && glm::length(route_[next_] - position) < kReached) ++next_;
    if (finished()) return glm::vec2(0.0f);
    const glm::vec2 toTarget = route_[next_] - position;
    const float length = glm::length(toTarget);
    return toTarget / length * std::min(distance, length);
}
