#include "Overlay.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

namespace {
constexpr int kAtlasColumns = 16;   // the font atlas holds ASCII 32..127, 16 characters per row
constexpr int kAtlasRows = 6;

// two triangles for one quad: (x, y, u, v) per corner
void addQuad(std::vector<float>& v, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1) {
    v.insert(v.end(), {x0, y0, u0, v0, x1, y0, u1, v0, x1, y1, u1, v1,
                       x0, y0, u0, v0, x1, y1, u1, v1, x0, y1, u0, v1});
}
}  // namespace

Overlay::Overlay(const std::string& assetRoot)
    : shader_(assetRoot + "/shaders/overlay.vert", assetRoot + "/shaders/overlay.frag"),
      font_(assetRoot + "/textures/font.png", false) {
    std::ifstream metrics(assetRoot + "/textures/font.txt");
    if (!(metrics >> cellW_ >> cellH_ >> padX_ >> padY_ >> advance_ >> lineHeight_))
        throw std::runtime_error("Cannot read " + assetRoot + "/textures/font.txt");

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

Overlay::~Overlay() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}

void Overlay::begin(int width, int height) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);   // the y axis is flipped, which flips the triangle winding
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_.use();
    shader_.setMat4("uProjection", glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f));
    shader_.setInt("uFont", 0);
    font_.bind(0);
}

void Overlay::end() {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Overlay::draw(const std::vector<float>& vertices, bool textured, const glm::vec4& color) {
    shader_.setBool("uTextured", textured);
    shader_.setVec4("uColor", color);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 4));
    glBindVertexArray(0);
}

void Overlay::rect(float x, float y, float w, float h, const glm::vec4& color) {
    std::vector<float> vertices;
    addQuad(vertices, x, y, x + w, y + h, 0.0f, 0.0f, 0.0f, 0.0f);
    draw(vertices, false, color);
}

float Overlay::textWidth(const std::string& s, float size) const {
    return static_cast<float>(s.size()) * advance_ * size / lineHeight_;
}

void Overlay::addText(std::vector<float>& vertices, const std::string& s, float x, float y, float size) const {
    const float scale = size / lineHeight_;
    const float atlasW = cellW_ * kAtlasColumns, atlasH = cellH_ * kAtlasRows;
    float pen = x;
    for (char ch : s) {
        int code = static_cast<unsigned char>(ch);
        if (code < 32 || code > 126) code = '?';
        const int index = code - 32, col = index % kAtlasColumns, row = index / kAtlasColumns;
        // the texture was flipped on load, so the atlas's top row is at v = 1
        const float u0 = col * cellW_ / atlasW, u1 = (col + 1) * cellW_ / atlasW;
        const float vTop = 1.0f - row * cellH_ / atlasH, vBottom = 1.0f - (row + 1) * cellH_ / atlasH;
        const float x0 = pen - padX_ * scale, y0 = y - padY_ * scale;
        addQuad(vertices, x0, y0, x0 + cellW_ * scale, y0 + cellH_ * scale, u0, vTop, u1, vBottom);
        pen += advance_ * scale;
    }
}

void Overlay::text(const std::string& s, float x, float y, float size, const glm::vec4& color, Align align,
                   bool shadow) {
    if (align == Align::Center) x -= textWidth(s, size) * 0.5f;
    else if (align == Align::Right) x -= textWidth(s, size);
    std::vector<float> vertices;
    if (shadow) {
        const float offset = size * 0.06f;
        addText(vertices, s, x + offset, y + offset, size);
        draw(vertices, true, glm::vec4(0.0f, 0.0f, 0.0f, color.a * 0.8f));
        vertices.clear();
    }
    addText(vertices, s, x, y, size);
    draw(vertices, true, color);
}
