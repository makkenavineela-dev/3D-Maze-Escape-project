#pragma once
#include "Camera.h"
#include "Flashlight.h"
#include "Maze.h"
#include "Mesh.h"
#include "Shader.h"

#include <string>

// Draws the sky, the ground and the maze walls, lit by the flashlight
class Scene {
public:
    explicit Scene(const std::string& shaderDir);

    void build(const Maze& maze);   // (re)creates the wall and ground meshes for a maze
    void draw(const Camera& camera, float aspect, const Flashlight& light) const;
    std::size_t wallTriangles() const { return walls_.triangleCount(); }

    float ambient = 0.05f;          // faint night vision outside the beam (near zero = dark)
    float fogDensity = 0.07f;       // how quickly distant surfaces fade to black

private:
    Shader sceneShader_;
    Shader skyShader_;
    Mesh walls_;
    Mesh ground_;
    Mesh sky_;
};

// Finds the "shaders" folder next to the program or in the current folder
std::string findShaderDir(const char* exePath);
