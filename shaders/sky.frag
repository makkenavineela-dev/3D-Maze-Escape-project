#version 330 core
// Night sky: a very dark blue glow at the horizon fading to black overhead

in vec3 vDirection;
out vec4 FragColor;

void main() {
    float height = clamp(normalize(vDirection).y, 0.0, 1.0);
    vec3 horizon = vec3(0.025, 0.03, 0.05);
    vec3 zenith = vec3(0.0, 0.0, 0.012);
    FragColor = vec4(mix(horizon, zenith, sqrt(height)), 1.0);
}
