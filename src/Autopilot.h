#pragma once
#include "Maze.h"

#include <glm/vec2.hpp>
#include <vector>

// Walks the solution from cell centre to cell centre and out of the exit.
// Used by the collision test and by the game's --autoplay demo.
class Autopilot {
public:
    explicit Autopilot(const Maze& maze);

    // Movement towards the next point of the route, at most `distance` long
    glm::vec2 step(glm::vec2 position, float distance);
    bool finished() const { return next_ >= route_.size(); }

private:
    std::vector<glm::vec2> route_;
    size_t next_ = 0;
};
