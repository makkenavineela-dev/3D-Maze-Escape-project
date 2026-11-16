#pragma once
#include "Shader.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

// 2D drawing on top of the 3D view: rectangles and text measured in screen pixels.
// An orthographic projection puts (0, 0) at the top-left corner of the window.
class Overlay {
public:
    enum class Align { Left, Center, Right };

    explicit Overlay(const std::string& assetRoot);
    ~Overlay();
    Overlay(const Overlay&) = delete;
    Overlay& operator=(const Overlay&) = delete;

    void begin(int width, int height);   // switches to 2D drawing with transparency
    void end();                          // back to 3D state

    void rect(float x, float y, float w, float h, const glm::vec4& color);
    // `size` is the line height in pixels; `y` is the top of the line
    void text(const std::string& s, float x, float y, float size, const glm::vec4& color,
              Align align = Align::Left, bool shadow = true);
    float textWidth(const std::string& s, float size) const;

private:
    void draw(const std::vector<float>& vertices, bool textured, const glm::vec4& color);
    void addText(std::vector<float>& vertices, const std::string& s, float x, float y, float size) const;

    Shader shader_;
    Texture font_;
    GLuint vao_ = 0, vbo_ = 0;
    // font atlas layout in atlas pixels, read from textures/font.txt
    float cellW_ = 0, cellH_ = 0, padX_ = 0, padY_ = 0, advance_ = 0, lineHeight_ = 0;
};
