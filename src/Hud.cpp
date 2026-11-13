#include "Hud.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
const glm::vec4 kWarmWhite(1.0f, 0.93f, 0.80f, 1.0f);
const glm::vec4 kGrey(0.72f, 0.72f, 0.72f, 1.0f);
const glm::vec4 kGreen(0.55f, 0.92f, 0.62f, 1.0f);

float pulse(double clock) { return 0.55f + 0.45f * static_cast<float>(std::sin(clock * 3.0)); }

glm::vec4 withAlpha(glm::vec4 color, float alpha) {
    color.a *= alpha;
    return color;
}
}  // namespace

std::string formatTime(double seconds) {
    const int total = static_cast<int>(seconds);
    char text[16];
    std::snprintf(text, sizeof(text), "%02d:%02d", total / 60, total % 60);
    return text;
}

void drawStartScreen(Overlay& overlay, int width, int height, double clock) {
    const float w = static_cast<float>(width), h = static_cast<float>(height), cx = w * 0.5f;
    overlay.rect(0, 0, w, h, glm::vec4(0.0f, 0.0f, 0.0f, 0.55f));
    overlay.text("THE DARK MAZE", cx, h * 0.22f, h * 0.11f, kWarmWhite, Overlay::Align::Center);
    overlay.text("Find the only way out of the circular maze", cx, h * 0.37f, h * 0.035f, kGrey, Overlay::Align::Center);
    overlay.text("Click or press ENTER to play", cx, h * 0.52f, h * 0.045f, withAlpha(kWarmWhite, pulse(clock)),
                 Overlay::Align::Center);
    const float small = h * 0.028f;
    overlay.text("Mouse: look    W A S D: walk    Shift: run    F: flashlight", cx, h * 0.78f, small, kGrey,
                 Overlay::Align::Center);
    overlay.text("1-6: lighting modes    F3: stats    F11: full screen    Esc: quit", cx, h * 0.78f + small * 1.6f,
                 small, kGrey, Overlay::Align::Center);
}

void drawVictoryScreen(Overlay& overlay, int width, int height, double escapeSeconds, double clock) {
    const float w = static_cast<float>(width), h = static_cast<float>(height), cx = w * 0.5f;
    overlay.rect(0, 0, w, h, glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));
    overlay.text("YOU ESCAPED!", cx, h * 0.25f, h * 0.11f, kGreen, Overlay::Align::Center);
    overlay.text("Time  " + formatTime(escapeSeconds), cx, h * 0.42f, h * 0.05f, kWarmWhite, Overlay::Align::Center);
    overlay.text("Click or press ENTER for a new maze", cx, h * 0.58f, h * 0.04f, withAlpha(kWarmWhite, pulse(clock)),
                 Overlay::Align::Center);
    overlay.text("Esc: quit", cx, h * 0.66f, h * 0.03f, kGrey, Overlay::Align::Center);
}

void drawPlayingHud(Overlay& overlay, int width, int height, const std::string& message, float messageAlpha) {
    const float w = static_cast<float>(width), h = static_cast<float>(height);
    // small crosshair in the middle of the screen
    const float arm = h * 0.012f, thick = std::max(1.0f, h * 0.0025f);
    const glm::vec4 crosshair(1.0f, 1.0f, 1.0f, 0.45f);
    overlay.rect(w * 0.5f - arm, h * 0.5f - thick * 0.5f, arm * 2.0f, thick, crosshair);
    overlay.rect(w * 0.5f - thick * 0.5f, h * 0.5f - arm, thick, arm * 2.0f, crosshair);

    if (!message.empty() && messageAlpha > 0.0f)
        overlay.text(message, w * 0.5f, h * 0.86f, h * 0.035f, withAlpha(kWarmWhite, messageAlpha), Overlay::Align::Center);
}

void drawStats(Overlay& overlay, int height, const std::string& stats) {
    const float h = static_cast<float>(height);
    overlay.text(stats, h * 0.02f, h * 0.02f, h * 0.028f, kGreen);
}
