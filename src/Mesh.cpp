#include "Mesh.h"

#include <cstddef>

void MeshData::addTriangle(const Vertex& a, const Vertex& b, const Vertex& c) {
    const auto base = static_cast<std::uint32_t>(vertices.size());
    const glm::vec3 faceNormal = glm::cross(b.position - a.position, c.position - a.position);
    const bool flip = glm::dot(faceNormal, a.normal + b.normal + c.normal) < 0.0f;
    vertices.insert(vertices.end(), {a, b, c});
    if (flip) indices.insert(indices.end(), {base, base + 2, base + 1});
    else      indices.insert(indices.end(), {base, base + 1, base + 2});
}

void MeshData::addQuad(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d) {
    const auto base = static_cast<std::uint32_t>(vertices.size());
    // cross product of the diagonals still works when one edge has zero length (sphere poles)
    const glm::vec3 faceNormal = glm::cross(c.position - a.position, d.position - b.position);
    const bool flip = glm::dot(faceNormal, a.normal + b.normal + c.normal + d.normal) < 0.0f;
    vertices.insert(vertices.end(), {a, b, c, d});
    if (flip) indices.insert(indices.end(), {base, base + 2, base + 1, base, base + 3, base + 2});
    else      indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
}

Mesh::~Mesh() { release(); }

void Mesh::release() {
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = ebo_ = 0;
    indexCount_ = 0;
}

void Mesh::upload(const MeshData& data) {
    release();
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.vertices.size() * sizeof(Vertex)),
                 data.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.indices.size() * sizeof(std::uint32_t)),
                 data.indices.data(), GL_STATIC_DRAW);

    // layout(location = 0) position, 1 normal, 2 texture coordinate
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    indexCount_ = static_cast<GLsizei>(data.indices.size());
}

void Mesh::draw() const {
    if (!vao_) return;
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
