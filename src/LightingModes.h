#pragma once

// Lighting demo modes. Keys 1-6 switch between them, so each part of the lighting
// formula can be shown on its own in the viva.
struct LightingMode {
    const char* name;
    bool flashlight;    // spotlight on
    bool cone;          // soft cone edge (off = light in every direction, like a bulb)
    bool attenuation;   // light gets weaker with distance
    bool fog;           // distance fog to black
    float ambient;      // light everywhere, even outside the beam
};

inline constexpr LightingMode kLightingModes[] = {
    {"1  Full lighting", true, true, true, true, 0.05f},
    {"2  No cone: light in every direction", true, false, true, true, 0.05f},
    {"3  No distance attenuation", true, true, false, true, 0.05f},
    {"4  No fog", true, true, true, false, 0.05f},
    {"5  Ambient only: flashlight off", false, true, true, true, 0.05f},
    {"6  Daylight: whole maze visible", false, true, true, false, 1.0f},
};
inline constexpr int kLightingModeCount = sizeof(kLightingModes) / sizeof(kLightingModes[0]);
