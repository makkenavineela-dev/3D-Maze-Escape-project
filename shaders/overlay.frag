#version 330 core
// 2D overlay: plain coloured rectangles, or text whose shape comes from the font texture's alpha

in vec2 vUV;

uniform sampler2D uFont;
uniform bool uTextured;
uniform vec4 uColor;

out vec4 FragColor;

void main() {
    float alpha = uTextured ? texture(uFont, vUV).a : 1.0;
    FragColor = vec4(uColor.rgb, uColor.a * alpha);
}
