#version 330 core
// Per-pixel flashlight lighting:
//   I = I_ambient + I_light * max(N.L, 0) * A_distance * A_cone
// multiplied by the surface texture, then distance fog blends the result towards black.

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

uniform sampler2D uTexture;    // wall or ground image

uniform bool  uLightOn;
uniform vec3  uLightPos;
uniform vec3  uLightDir;       // beam axis, normalised
uniform vec3  uLightColor;     // colour x intensity
uniform float uCosInner;       // cos of the angle where the soft edge starts
uniform float uCosOuter;       // cos of the angle where the beam ends
uniform float uAttConstant;
uniform float uAttLinear;
uniform float uAttQuadratic;
uniform bool  uUseCone;        // demo switches: turn single terms off to show what they do
uniform bool  uUseAttenuation;
uniform float uAmbient;
uniform float uFogDensity;
uniform vec3  uCameraPos;
uniform bool  uEmissive;
uniform vec4  uEmissiveColor;

out vec4 FragColor;

void main() {
    if (uEmissive) {
        FragColor = uEmissiveColor;
        return;
    }

    vec3 light = vec3(0.0);
    if (uLightOn) {
        vec3 N = normalize(vNormal);
        vec3 toLight = uLightPos - vWorldPos;
        float dist = length(toLight);
        vec3 L = toLight / dist;

        // Diffuse: surfaces facing the light are brightest
        float diffuse = max(dot(N, L), 0.0);

        // Distance attenuation: light gets weaker further from the torch
        float attenuation = uUseAttenuation
            ? 1.0 / (uAttConstant + uAttLinear * dist + uAttQuadratic * dist * dist)
            : 1.0;

        // Cone: 1 inside the inner angle, smooth fade to 0 at the outer angle
        float theta = dot(-L, uLightDir);
        float cone = uUseCone ? smoothstep(uCosOuter, uCosInner, theta) : 1.0;
        light = uLightColor * diffuse * attenuation * cone;
    }
    vec3 albedo = texture(uTexture, vUV).rgb;
    vec3 color = albedo * (uAmbient + light);

    // Fog: the further away a surface is, the more it fades to black (exponential squared)
    float visibility = 1.0;
    if (uFogDensity > 0.0) {
        vec3 toCamera = uCameraPos - vWorldPos;
        float fogAmount = uFogDensity * uFogDensity * dot(toCamera, toCamera);
        visibility = exp(-fogAmount);
    }
    FragColor = vec4(color * visibility, 1.0);
}
