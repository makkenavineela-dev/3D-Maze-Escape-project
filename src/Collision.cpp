#include "Collision.h"
#include "Config.h"

#include <algorithm>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

namespace {

constexpr float kTwoPi = glm::two_pi<float>();
constexpr float kMaxStep = 0.1f;   // long moves are split so a fast player can never jump through a wall
constexpr int kIterations = 4;     // repeated push-outs settle corners where two walls meet

float wrapAngle(float a) {
    a = std::fmod(a, kTwoPi);
    return a < 0.0f ? a + kTwoPi : a;
}

float angleBetween(float a, float b) {
    const float d = std::fabs(a - b);
    return std::min(d, kTwoPi - d);
}

glm::vec2 direction(float angle) { return {std::cos(angle), std::sin(angle)}; }

// Closest point to p on the centre line of a wall. The wall is then treated as that line
// thickened by half the wall thickness (a capsule), which matches the drawn walls closely.
glm::vec2 closestPoint(const WallSegment& wall, glm::vec2 p) {
    const float half = cfg::kWallThickness * 0.5f;
    if (wall.type == WallSegment::Type::Radial) {
        const glm::vec2 d = direction(wall.angleStart);
        return d * std::clamp(glm::dot(p, d), wall.radiusInner, wall.radiusOuter);
    }
    // Arc: shorten by half a thickness at each end, so the rounded end lines up with the flat end cap
    const float radius = wall.radiusInner;
    const float trim = half / radius;
    const float a0 = wall.angleStart + trim, a1 = wall.angleEnd - trim;
    const float angle = wrapAngle(std::atan2(p.y, p.x));
    if (angle >= a0 && angle <= a1) return direction(angle) * radius;
    return direction(angleBetween(angle, a0) < angleBetween(angle, a1) ? a0 : a1) * radius;
}

}  // namespace

Collision::Collision(const Maze& maze) : maze_(&maze), walls_(maze.walls()) {
    // Which cells each wall borders: look just either side of the middle of the wall
    std::vector<std::vector<std::vector<int>>> own(maze.rings());
    for (int r = 0; r < maze.rings(); ++r) own[r].resize(maze.sectors(r));
    auto addIfInside = [&](int wall, glm::vec2 probe) {
        const CellId cell = maze.worldToCell(probe.x, probe.y);
        if (cell.ring < maze.rings()) own[cell.ring][cell.sector].push_back(wall);
    };
    for (int i = 0; i < static_cast<int>(walls_.size()); ++i) {
        const WallSegment& w = walls_[i];
        if (w.type == WallSegment::Type::Arc) {
            const glm::vec2 d = direction((w.angleStart + w.angleEnd) * 0.5f);
            addIfInside(i, d * (w.radiusInner - 0.01f));
            addIfInside(i, d * (w.radiusInner + 0.01f));
        } else {
            const float middle = (w.radiusInner + w.radiusOuter) * 0.5f;
            addIfInside(i, direction(w.angleStart - 0.001f) * middle);
            addIfInside(i, direction(w.angleStart + 0.001f) * middle);
        }
    }

    // A player in a cell can only reach walls of that cell or of the cells next to it
    nearby_.resize(maze.rings());
    for (int r = 0; r < maze.rings(); ++r) {
        nearby_[r].resize(maze.sectors(r));
        for (int s = 0; s < maze.sectors(r); ++s) {
            std::vector<int>& list = nearby_[r][s];
            list = own[r][s];
            for (CellId n : maze.neighbours({r, s})) list.insert(list.end(), own[n.ring][n.sector].begin(), own[n.ring][n.sector].end());
            std::sort(list.begin(), list.end());
            list.erase(std::unique(list.begin(), list.end()), list.end());
        }
    }
}

const std::vector<int>& Collision::wallsNear(glm::vec2 p) const {
    CellId cell = maze_->worldToCell(p.x, p.y);
    if (cell.ring >= maze_->rings()) {   // outside the maze: use the outer-ring cell at the same angle
        const int ring = maze_->rings() - 1;
        const int count = maze_->sectors(ring);
        cell = {ring, std::min(static_cast<int>(wrapAngle(std::atan2(p.y, p.x)) / kTwoPi * count), count - 1)};
    }
    return nearby_[cell.ring][cell.sector];
}

glm::vec2 Collision::resolve(glm::vec2 p) const {
    const float minDistance = cfg::kPlayerRadius + cfg::kWallThickness * 0.5f;
    for (int iteration = 0; iteration < kIterations; ++iteration) {
        bool pushed = false;
        for (int i : wallsNear(p)) {
            const glm::vec2 closest = closestPoint(walls_[i], p);
            const glm::vec2 away = p - closest;
            const float distance = glm::length(away);
            if (distance >= minDistance) continue;
            // push out along the line from the wall to the player (any direction if exactly on the line)
            const glm::vec2 normal = distance > 1e-5f ? away / distance : glm::normalize(p + glm::vec2(1e-3f, 0.0f));
            p = closest + normal * minDistance;
            pushed = true;
        }
        if (!pushed) break;
    }
    return p;
}

glm::vec2 Collision::move(glm::vec2 from, glm::vec2 delta) const {
    const int steps = std::max(1, static_cast<int>(std::ceil(glm::length(delta) / kMaxStep)));
    glm::vec2 p = from;
    for (int i = 0; i < steps; ++i) p = resolve(p + delta / static_cast<float>(steps));
    return p;
}

float Collision::clearance(glm::vec2 p) const {
    float nearest = 1e9f;
    for (int i : wallsNear(p)) nearest = std::min(nearest, glm::length(p - closestPoint(walls_[i], p)));
    return nearest - cfg::kWallThickness * 0.5f - cfg::kPlayerRadius;
}
