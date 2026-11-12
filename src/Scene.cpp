#include "Scene.h"
#include "Config.h"
#include "MazeMesh.h"

#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <vector>

namespace {
const glm::vec3 kWallColor(0.55f, 0.52f, 0.48f);     // pale stone
const glm::vec3 kGroundColor(0.33f, 0.30f, 0.27f);   // dark earth
}

Scene::Scene(const std::string& shaderDir)
    : sceneShader_(shaderDir + "/scene.vert", shaderDir + "/scene.frag"),
      skyShader_(shaderDir + "/sky.vert", shaderDir + "/sky.frag") {
    sky_.upload(buildSkyMesh());
}

void Scene::build(const Maze& maze) {
    walls_.upload(buildWallMesh(maze.walls()));
    ground_.upload(buildGroundMesh(maze.outerRadius() + cfg::kGroundMargin));
}

void Scene::draw(const Camera& camera, float aspect, const Flashlight& light) const {
    const glm::mat4 view = camera.view();
    const glm::mat4 projection = camera.projection(aspect);

    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Sky first, behind everything: it turns with the camera but never moves (translation removed)
    glDisable(GL_DEPTH_TEST);
    skyShader_.use();
    skyShader_.setMat4("uView", glm::mat4(glm::mat3(view)));
    skyShader_.setMat4("uProjection", projection);
    sky_.draw();
    glEnable(GL_DEPTH_TEST);

    // Ground and walls with the flashlight
    sceneShader_.use();
    sceneShader_.setMat4("uModel", glm::mat4(1.0f));          // the maze is built directly in world space
    sceneShader_.setMat3("uNormalMatrix", glm::mat3(1.0f));
    sceneShader_.setMat4("uView", view);
    sceneShader_.setMat4("uProjection", projection);
    sceneShader_.setBool("uLightOn", light.on);
    sceneShader_.setVec3("uLightPos", light.position);
    sceneShader_.setVec3("uLightDir", glm::normalize(light.direction));
    sceneShader_.setVec3("uLightColor", light.color * light.intensity);
    sceneShader_.setFloat("uCosInner", std::cos(glm::radians(light.innerConeDegrees)));
    sceneShader_.setFloat("uCosOuter", std::cos(glm::radians(light.outerConeDegrees)));
    sceneShader_.setFloat("uAttConstant", light.constant);
    sceneShader_.setFloat("uAttLinear", light.linear);
    sceneShader_.setFloat("uAttQuadratic", light.quadratic);
    sceneShader_.setFloat("uAmbient", ambient);
    sceneShader_.setFloat("uFogDensity", fogDensity);
    sceneShader_.setVec3("uCameraPos", camera.position);

    sceneShader_.setVec3("uBaseColor", kGroundColor);
    ground_.draw();
    sceneShader_.setVec3("uBaseColor", kWallColor);
    walls_.draw();
}

std::string findShaderDir(const char* exePath) {
    namespace fs = std::filesystem;
    std::error_code error;
    const fs::path exeDir = fs::absolute(fs::path(exePath), error).parent_path();
    const std::vector<fs::path> candidates = {exeDir / "shaders", fs::current_path(error) / "shaders",
                                              exeDir.parent_path() / "shaders"};
    for (const fs::path& dir : candidates)
        if (fs::exists(dir / "scene.vert", error)) return dir.string();
    throw std::runtime_error("Cannot find the 'shaders' folder (looked next to the program and in the current folder)");
}
