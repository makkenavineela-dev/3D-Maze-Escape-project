#include "Scene.h"
#include "Config.h"
#include "MazeMesh.h"

#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <vector>

Scene::Scene(const std::string& assetRoot)
    : sceneShader_(assetRoot + "/shaders/scene.vert", assetRoot + "/shaders/scene.frag"),
      skyShader_(assetRoot + "/shaders/sky.vert", assetRoot + "/shaders/sky.frag"),
      wallTexture_(assetRoot + "/textures/wall.png"),
      groundTexture_(assetRoot + "/textures/ground.png") {
    sky_.upload(buildSkyMesh());
}

void Scene::build(const Maze& maze) {
    walls_.upload(buildWallMesh(maze.walls()));
    ground_.upload(buildGroundMesh(maze.outerRadius() + cfg::kGroundMargin));
    exitTrigger_.upload(buildExitTriggerMesh(maze));
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
    sceneShader_.setBool("uUseCone", useCone);
    sceneShader_.setBool("uUseAttenuation", useAttenuation);
    sceneShader_.setFloat("uAmbient", ambient);
    sceneShader_.setFloat("uFogDensity", fogDensity);
    sceneShader_.setVec3("uCameraPos", camera.position);
    sceneShader_.setInt("uTexture", 0);

    groundTexture_.bind(0);
    ground_.draw();

    // Draw the exit's trigger band as a translucent, emissive green arc.
    sceneShader_.setBool("uEmissive", true);
    sceneShader_.setVec4("uEmissiveColor", glm::vec4(0.1f, 1.0f, 0.15f, 0.45f));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    exitTrigger_.draw();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    sceneShader_.setBool("uEmissive", false);

    wallTexture_.bind(0);
    walls_.draw();
}

std::string findAssetRoot(const char* exePath) {
    namespace fs = std::filesystem;
    std::error_code error;
    const fs::path exeDir = fs::absolute(fs::path(exePath), error).parent_path();
    for (const fs::path& dir : {exeDir, fs::current_path(error), exeDir.parent_path()})
        if (fs::exists(dir / "shaders" / "scene.vert", error) && fs::exists(dir / "textures" / "wall.png", error))
            return dir.string();
    throw std::runtime_error("Cannot find the 'shaders' and 'textures' folders (looked next to the program and in the current folder)");
}
