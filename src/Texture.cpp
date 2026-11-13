#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstring>
#include <stdexcept>

namespace {

// Anisotropic filtering keeps the ground sharp when seen at a low angle. It is an extension in
// OpenGL 3.3 (core only from 4.6), so check for it and use the extension's constant values.
constexpr GLenum kTextureMaxAnisotropy = 0x84FE;
constexpr GLenum kMaxTextureMaxAnisotropy = 0x84FF;

bool anisotropySupported() {
    static const bool supported = [] {
        GLint count = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (GLint i = 0; i < count; ++i) {
            const char* name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
            if (name && (std::strcmp(name, "GL_EXT_texture_filter_anisotropic") == 0 ||
                         std::strcmp(name, "GL_ARB_texture_filter_anisotropic") == 0))
                return true;
        }
        return false;
    }();
    return supported;
}

}  // namespace

Texture::Texture(const std::string& path, bool repeat) {
    stbi_set_flip_vertically_on_load(true);   // images start at the top row, OpenGL textures at the bottom
    int channels = 0;
    unsigned char* pixels = stbi_load(path.c_str(), &width_, &height_, &channels, 4);
    if (!pixels) throw std::runtime_error("Cannot load texture " + path + ": " + stbi_failure_reason());

    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    const GLint wrap = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (anisotropySupported()) {
        GLfloat maxAnisotropy = 1.0f;
        glGetFloatv(kMaxTextureMaxAnisotropy, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, kTextureMaxAnisotropy, maxAnisotropy < 8.0f ? maxAnisotropy : 8.0f);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if (id_) glDeleteTextures(1, &id_);
}

void Texture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
    glBindTexture(GL_TEXTURE_2D, id_);
}
