#pragma once
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Autopilot.h"
#include "Camera.h"
#include "Collision.h"
#include "Controls.h"
#include "Flashlight.h"
#include "Maze.h"
#include "Overlay.h"
#include "Scene.h"

#include <optional>
#include <random>
#include <string>

// Game flow (Person 1): start screen -> playing -> victory -> new maze -> playing ...
class Game {
public:
    // autoplay = demo mode: the game walks itself out of each maze and starts the next one
    Game(GLFWwindow* window, const std::string& assetRoot, unsigned firstSeed, bool autoplay);
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void run(int maxFrames);   // main loop; maxFrames > 0 stops after that many frames
    void onVictory();          // called when the exit check sees the player outside

private:
    enum class State { Start, Playing, Victory };

    void newMaze(unsigned seed);
    void startPlaying();
    void playNewMaze();
    void update(float dt);
    void render(int width, int height);
    void applyLighting();
    void onKey(int key);
    void onClick();
    void showMessage(const std::string& text);
    void toggleFullscreen();

    GLFWwindow* window_;
    bool autoplay_;
    std::mt19937 rng_;
    Maze maze_;
    Collision collision_;
    std::optional<Autopilot> autopilot_;
    Scene scene_;
    Overlay overlay_;
    Camera camera_;
    Controls controls_;
    Flashlight flashlight_;

    State state_ = State::Start;
    double clock_ = 0.0;          // seconds since the game started
    double playSeconds_ = 0.0;    // time spent in the current maze
    double victorySeconds_ = 0.0;
    double victoryClock_ = 0.0;
    int lightingMode_ = 0;        // index into kLightingModes
    bool flashlightOn_ = true;
    float fogDensity_;
    bool showStats_ = false;
    int fps_ = 0;
    std::string message_;
    double messageUntil_ = 0.0;
    int windowedX_ = 100, windowedY_ = 100, windowedW_ = 1280, windowedH_ = 720;
};
