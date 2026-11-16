#pragma once
#include "Overlay.h"

#include <string>

// The game's screens, drawn with the overlay. `clock` (seconds) animates pulsing text.
void drawStartScreen(Overlay& overlay, int width, int height, double clock);
void drawVictoryScreen(Overlay& overlay, int width, int height, double escapeSeconds, double clock);
// While playing: crosshair and an optional fading message
void drawPlayingHud(Overlay& overlay, int width, int height, const std::string& message, float messageAlpha);
// Small stats line in the top-left corner (FPS, maze number, time)
void drawStats(Overlay& overlay, int height, const std::string& stats);

std::string formatTime(double seconds);   // "02:31"
