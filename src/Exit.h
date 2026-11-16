
#pragma once

#include "Config.h"
#include "Maze.h"

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

#include <cmath>

// Exit detection.
// Creates an arc-shaped trigger zone outside the maze exit.
// The player wins when they enter this curved trigger zone.
inline bool hasEscaped(const Maze& maze, glm::vec2 position) {

    // Centre of the exit gap on the outer wall
    const glm::vec2 exitWall = maze.exitWorldPos();

    // Distance from maze centre to the exit wall
    const float outerRadius = glm::length(exitWall);

    // Prevent division by zero
    if (outerRadius < 0.0001f)
        return false;

    // Direction pointing outward from the maze centre
    const glm::vec2 exitDir = exitWall / outerRadius;

    // Player's distance from maze centre
    const float playerRadius = glm::length(position);

    if (playerRadius < 0.0001f)
        return false;

    // Player's direction from maze centre
    const glm::vec2 playerDir = position / playerRadius;

    // Calculate angular difference between player and exit direction
    const float cross =
        exitDir.x * playerDir.y - exitDir.y * playerDir.x;

    const float dot = glm::dot(exitDir, playerDir);

    const float angleDifference =
        std::abs(std::atan2(cross, dot));

    // Centre of the arc trigger, positioned outside the maze wall
    const float triggerRadius =
        outerRadius + cfg::kExitTriggerOffset;

    // Check whether the player is inside the radial thickness of the arc
    const bool insideRadius =
        std::abs(playerRadius - triggerRadius)
        <= cfg::kExitTriggerThickness * 0.5f;

    // Check whether the player is within the angular span of the exit
    const bool insideAngle =
        angleDifference <= cfg::kExitTriggerHalfAngle;

    // Both conditions must be satisfied to win
    return insideRadius && insideAngle;
}
