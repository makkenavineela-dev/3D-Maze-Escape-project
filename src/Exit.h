#pragma once
#include "Config.h"
#include "Maze.h"

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

// Exit detection (Person 4). The outer wall blocks every way out except the exit gap, so a player
// who is a few steps outside the outer wall must have walked out through the exit.
inline bool hasEscaped(const Maze& maze, glm::vec2 position) {
    return glm::length(position) > maze.outerRadius() + cfg::kExitDistance;
}
