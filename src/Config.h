
#pragma once

// Shared sizes and settings agreed on at the day-1 kickoff. Every module reads them from here,
// so changing a value in one place changes the whole game.
namespace cfg {

// Maze layout
constexpr int   kRings             = 8;
constexpr int   kInnerSectors      = 6;
constexpr float kMaxCellWidthRatio = 2.0f;
constexpr float kRingWidth         = 3.0f;

// Walls and ground
constexpr float kWallHeight    = 3.0f;
constexpr float kWallThickness = 0.25f;
constexpr float kGroundMargin  = 30.0f;

// Collision and exit (Persons 2 and 4)
constexpr float kPlayerRadius = 0.3f;

// Arc-shaped exit trigger settings
constexpr float kExitTriggerOffset    = 0.5f;  // Trigger band starts at the outer wall
constexpr float kExitTriggerThickness = 1.0f;  // Radial thickness of the trigger band
constexpr float kExitTriggerHalfAngle = 0.15f; // Angular half-width in radians

// Player and camera
constexpr float kEyeHeight        = 1.6f;
constexpr float kWalkSpeed        = 3.0f;
constexpr float kSprintFactor     = 1.8f;
constexpr float kMouseSensitivity = 0.1f;
constexpr float kFovDegrees       = 70.0f;

}  // namespace cfg
