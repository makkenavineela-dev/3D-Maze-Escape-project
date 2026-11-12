#version 330 core
// Sky dome: a unit sphere around the camera. uView has no translation, so the sky never gets closer.

layout(location = 0) in vec3 aPosition;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vDirection;

void main() {
    vDirection = aPosition;
    gl_Position = uProjection * uView * vec4(aPosition, 1.0);
}
