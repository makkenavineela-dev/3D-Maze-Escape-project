#pragma once

// Shared sizes and settings agreed at the day-1 kickoff. Every module reads them from here,
// so changing a value in one place changes the whole game.
namespace cfg {

// Maze layout (Person 2)
constexpr int   kRings             = 8;     // ring 0 is the round centre room
constexpr int   kInnerSectors      = 6;     // cells in ring 1
constexpr float kMaxCellWidthRatio = 2.0f;  // a ring doubles its sectors when cells get wider than this x ring width
constexpr float kRingWidth         = 3.0f;  // metres between two ring walls

// Walls and ground (Person 3)
constexpr float kWallHeight    = 3.0f;
constexpr float kWallThickness = 0.25f;
constexpr float kGroundMargin  = 30.0f;     // ground continues this far past the outer wall

// Collision and exit (Persons 2 and 4)
constexpr float kPlayerRadius = 0.3f;   // the player is a circle this wide (metres) when touching walls
constexpr float kExitDistance = 1.5f;   // the player wins this far outside the outer wall

// Player and camera (Person 1)
constexpr float kEyeHeight        = 1.6f;
constexpr float kWalkSpeed        = 3.0f;   // metres per second
constexpr float kSprintFactor     = 1.8f;   // speed multiplier while Shift is held
constexpr float kMouseSensitivity = 0.1f;   // degrees per pixel of mouse movement
constexpr float kFovDegrees       = 70.0f;

}  // namespace cfg
