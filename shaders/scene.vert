#version 330 core
// Ground and walls (Person 4): passes world-space position and normal to the fragment shader

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat3 uNormalMatrix;   // keeps normals correct when the model matrix scales or rotates
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    vWorldPos = world.xyz;
    vNormal = uNormalMatrix * aNormal;
    vUV = aUV;
    gl_Position = uProjection * uView * world;
}
