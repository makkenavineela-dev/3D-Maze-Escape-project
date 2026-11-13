#pragma once
#include <glad/glad.h>
#include <string>

// An image file loaded onto the GPU with stb_image, with mipmaps (Person 3)
class Texture {
public:
    // repeat = true tiles the image (walls, ground); false clamps at the edges (font)
    explicit Texture(const std::string& path, bool repeat = true);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(int unit = 0) const;
    int width() const { return width_; }
    int height() const { return height_; }

private:
    GLuint id_ = 0;
    int width_ = 0;
    int height_ = 0;
};
