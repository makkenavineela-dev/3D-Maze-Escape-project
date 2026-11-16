#pragma once
#include "Camera.h"
#include "Flashlight.h"
#include "Maze.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

#include <string>

// Draws the sky, the textured ground and maze walls, lit by the flashlight
class Scene {
public:
    explicit Scene(const std::string& assetRoot);   // folder that holds shaders/ and textures/

    void build(const Maze& maze);   // (re)creates the wall and ground meshes for a maze
    void draw(const Camera& camera, float aspect, const Flashlight& light) const;
    std::size_t wallTriangles() const { return walls_.triangleCount(); }

    float ambient = 0.05f;          // faint night vision outside the beam (near zero = dark)
    float fogDensity = 0.07f;       // how quickly distant surfaces fade to black
    bool useCone = true;            // lighting demo switches (see LightingModes.h)
    bool useAttenuation = true;

private:
    Shader sceneShader_;
    Shader skyShader_;
    Texture wallTexture_;
    Texture groundTexture_;
    Mesh walls_;
    Mesh ground_;
    Mesh exitTrigger_;
    Mesh sky_;
};

// Finds the folder with "shaders" and "textures": next to the program or in the current folder
std::string findAssetRoot(const char* exePath);
