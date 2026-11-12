#version 330 core
// Flashlight lighting per pixel (Person 4):
//   I = I_ambient + I_light * max(N.L, 0) * A_distance * A_cone
// then distance fog blends the result towards black.

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

uniform vec3 uBaseColor;

uniform bool  uLightOn;
uniform vec3  uLightPos;
uniform vec3  uLightDir;       // beam axis, normalised
uniform vec3  uLightColor;     // colour x intensity
uniform float uCosInner;       // cos of the angle where the soft edge starts
uniform float uCosOuter;       // cos of the angle where the beam ends
uniform float uAttConstant;
uniform float uAttLinear;
uniform float uAttQuadratic;
uniform float uAmbient;
uniform float uFogDensity;
uniform vec3  uCameraPos;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 toLight = uLightPos - vWorldPos;
    float dist = length(toLight);
    vec3 L = toLight / dist;

    // Diffuse: surfaces facing the light are brightest
    float diffuse = max(dot(N, L), 0.0);

    // Distance attenuation: light gets weaker further from the torch
    float attenuation = 1.0 / (uAttConstant + uAttLinear * dist + uAttQuadratic * dist * dist);

    // Cone: 1 inside the inner angle, smooth fade to 0 at the outer angle
    float theta = dot(-L, uLightDir);
    float cone = smoothstep(uCosOuter, uCosInner, theta);

    vec3 light = uLightOn ? uLightColor * diffuse * attenuation * cone : vec3(0.0);
    vec3 color = uBaseColor * (uAmbient + light);

    // Fog: the further away a surface is, the more it fades to black (exponential squared)
    float fogDistance = length(uCameraPos - vWorldPos);
    float visibility = exp(-pow(uFogDensity * fogDistance, 2.0));
    FragColor = vec4(color * visibility, 1.0);
}
