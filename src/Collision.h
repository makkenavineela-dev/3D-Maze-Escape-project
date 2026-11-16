#pragma once
#include "Maze.h"

#include <glm/vec2.hpp>
#include <vector>

// Wall collision. The player is a circle on the ground plane (x, z). Each frame only the
// walls of the player's cell and its neighbouring cells are tested. When the circle overlaps a wall
// it is pushed straight out, which cancels the part of the movement going into the wall and keeps
// the part along it, so the player slides along walls instead of sticking.
class Collision {
public:
    explicit Collision(const Maze& maze);

    // Moves the player from `from` by `delta` and returns where they end up
    glm::vec2 move(glm::vec2 from, glm::vec2 delta) const;

    // Distance from p to the nearest wall surface minus the player radius (negative = overlapping)
    float clearance(glm::vec2 p) const;

private:
    glm::vec2 resolve(glm::vec2 p) const;
    const std::vector<int>& wallsNear(glm::vec2 p) const;

    const Maze* maze_;
    std::vector<WallSegment> walls_;
    std::vector<std::vector<std::vector<int>>> nearby_;   // [ring][sector] -> walls of that cell and its neighbours
};
