#version 330 core
// 2D overlay: screen-pixel positions are turned into clip space by an orthographic projection

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;

uniform mat4 uProjection;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
}
