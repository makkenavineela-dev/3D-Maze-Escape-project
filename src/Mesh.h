#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

// Vertices and indices built on the CPU before they are uploaded to the GPU
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    // Corners are given in order around the edge. The winding is fixed automatically so the
    // front face points the same way as the vertex normals (needed for back-face culling).
    void addTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
    void addQuad(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d);
};

// A mesh stored on the GPU: vertex array (VAO), vertex buffer (VBO) and index buffer (EBO)
class Mesh {
public:
    Mesh() = default;
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void upload(const MeshData& data);   // replaces any previous contents
    void draw() const;
    std::size_t triangleCount() const { return static_cast<std::size_t>(indexCount_) / 3; }

private:
    void release();

    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    GLsizei indexCount_ = 0;
};
