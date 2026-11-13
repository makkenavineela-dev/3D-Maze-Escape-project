#include "Game.h"
#include "Config.h"
#include "Exit.h"
#include "Hud.h"
#include "LightingModes.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace {
constexpr double kMessageSeconds = 2.5;           // how long a message like "Flashlight off" stays
constexpr float kStartScreenTurnSpeed = 6.0f;     // degrees per second the view drifts on the start screen
constexpr float kAutoplayTurnSpeed = 240.0f;      // degrees per second the autopilot can turn
constexpr double kAutoplayNextMazeSeconds = 3.0;  // autoplay shows the victory screen this long

float wrapDegrees(float a) {
    a = std::fmod(a + 180.0f, 360.0f);
    return (a < 0.0f ? a + 360.0f : a) - 180.0f;
}
}  // namespace

Game::Game(GLFWwindow* window, const std::string& assetRoot, unsigned firstSeed, bool autoplay)
    : window_(window), autoplay_(autoplay), rng_(std::random_device{}()), maze_(firstSeed), collision_(maze_),
      scene_(assetRoot), overlay_(assetRoot), camera_(glm::vec3(0.0f, cfg::kEyeHeight, 0.0f)),
      controls_(window, camera_), fogDensity_(scene_.fogDensity) {
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS) static_cast<Game*>(glfwGetWindowUserPointer(w))->onKey(key);
    });
    glfwSetMouseButtonCallback(window_, [](GLFWwindow* w, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) static_cast<Game*>(glfwGetWindowUserPointer(w))->onClick();
    });

    newMaze(firstSeed);
    if (autoplay_) startPlaying();
}

// Builds everything for a maze and puts the player in the centre, facing one of its doorways
void Game::newMaze(unsigned seed) {
    maze_ = Maze(seed);
    collision_ = Collision(maze_);
    scene_.build(maze_);
    if (autoplay_) autopilot_.emplace(maze_);

    std::vector<CellId> doors;
    for (CellId n : maze_.neighbours({0, 0}))
        if (maze_.linked({0, 0}, n)) doors.push_back(n);
    std::uniform_int_distribution<size_t> pick(0, doors.size() - 1);
    const glm::vec2 door = maze_.cellCentre(doors[pick(rng_)]);
    camera_ = Camera(glm::vec3(0.0f, cfg::kEyeHeight, 0.0f), glm::degrees(std::atan2(door.y, door.x)));

    std::printf("Maze %u: %d cells, exit in outer sector %d\n", seed, maze_.cellCount(), maze_.exitSector());
    std::fflush(stdout);
}

void Game::startPlaying() {
    state_ = State::Playing;
    playSeconds_ = 0.0;
    controls_.setCaptured(!autoplay_);
}

void Game::playNewMaze() {
    newMaze(rng_());
    startPlaying();
}

void Game::onVictory() {
    state_ = State::Victory;
    victorySeconds_ = playSeconds_;
    victoryClock_ = clock_;
    controls_.setCaptured(false);
    std::printf("Escaped maze %u in %s\n", maze_.seed(), formatTime(victorySeconds_).c_str());
    std::fflush(stdout);
}

void Game::update(float dt) {
    clock_ += dt;
    if (state_ == State::Start) {
        camera_.rotate(kStartScreenTurnSpeed * dt, 0.0f);   // the flashlight slowly sweeps the centre room
    } else if (state_ == State::Playing) {
        playSeconds_ += dt;
        const glm::vec2 position(camera_.position.x, camera_.position.z);
        glm::vec2 move = controls_.update(dt);
        if (autopilot_) {
            move = autopilot_->step(position, cfg::kWalkSpeed * cfg::kSprintFactor * dt);
            if (glm::length(move) > 0.0f) {   // turn smoothly towards where it walks
                const float turn = wrapDegrees(glm::degrees(std::atan2(move.y, move.x)) - camera_.yaw());
                const float maxTurn = kAutoplayTurnSpeed * dt;
                camera_.rotate(std::clamp(turn, -maxTurn, maxTurn), -camera_.pitch());
            }
        }
        const glm::vec2 next = collision_.move(position, move);   // walls stop the player here
        camera_.position = glm::vec3(next.x, cfg::kEyeHeight, next.y);
        if (hasEscaped(maze_, next)) onVictory();
    } else if (autoplay_ && clock_ - victoryClock_ > kAutoplayNextMazeSeconds) {
        playNewMaze();
    }
    flashlight_.follow(camera_);
    applyLighting();
}

void Game::applyLighting() {
    const LightingMode& mode = kLightingModes[lightingMode_];
    flashlight_.on = mode.flashlight && flashlightOn_;
    scene_.useCone = mode.cone;
    scene_.useAttenuation = mode.attenuation;
    scene_.fogDensity = mode.fog ? fogDensity_ : 0.0f;
    scene_.ambient = mode.ambient;
}

void Game::render(int width, int height) {
    glViewport(0, 0, width, height);
    scene_.draw(camera_, static_cast<float>(width) / static_cast<float>(height), flashlight_);

    overlay_.begin(width, height);
    if (state_ == State::Start) {
        drawStartScreen(overlay_, width, height, clock_);
    } else if (state_ == State::Victory) {
        drawVictoryScreen(overlay_, width, height, victorySeconds_, clock_);
    } else {
        const float messageAlpha = static_cast<float>(std::clamp((messageUntil_ - clock_) / 0.5, 0.0, 1.0));
        drawPlayingHud(overlay_, width, height, message_, messageAlpha);
    }
    if (showStats_) {
        char stats[96];
        std::snprintf(stats, sizeof(stats), "FPS %d   Maze %u   Time %s", fps_, maze_.seed(), formatTime(playSeconds_).c_str());
        drawStats(overlay_, height, stats);
    }
    overlay_.end();
}

void Game::showMessage(const std::string& text) {
    message_ = text;
    messageUntil_ = clock_ + kMessageSeconds;
}

void Game::onKey(int key) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
        break;
    case GLFW_KEY_ENTER:
    case GLFW_KEY_KP_ENTER:
    case GLFW_KEY_SPACE:
        if (state_ == State::Start) startPlaying();
        else if (state_ == State::Victory) playNewMaze();
        break;
    case GLFW_KEY_R:   // give up and try a new maze
        if (state_ != State::Start) playNewMaze();
        break;
    case GLFW_KEY_F:
        flashlightOn_ = !flashlightOn_;
        showMessage(flashlightOn_ ? "Flashlight on" : "Flashlight off");
        break;
    case GLFW_KEY_F3:
        showStats_ = !showStats_;
        break;
    case GLFW_KEY_F11:
        toggleFullscreen();
        break;
    default:
        if (key >= GLFW_KEY_1 && key < GLFW_KEY_1 + kLightingModeCount) {
            lightingMode_ = key - GLFW_KEY_1;
            showMessage(kLightingModes[lightingMode_].name);
        }
    }
}

void Game::onClick() {
    if (state_ == State::Start) startPlaying();
    else if (state_ == State::Victory) playNewMaze();
}

void Game::toggleFullscreen() {
    if (glfwGetWindowMonitor(window_)) {
        glfwSetWindowMonitor(window_, nullptr, windowedX_, windowedY_, windowedW_, windowedH_, 0);
    } else {
        glfwGetWindowPos(window_, &windowedX_, &windowedY_);
        glfwGetWindowSize(window_, &windowedW_, &windowedH_);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
}

void Game::run(int maxFrames) {
    const double startTime = glfwGetTime();
    double lastTime = startTime, fpsTime = lastTime;
    int frames = 0, fpsFrames = 0;
    while (!glfwWindowShouldClose(window_)) {
        const double now = glfwGetTime();
        const float dt = std::min(static_cast<float>(now - lastTime), 0.1f);   // no big jumps after a pause
        lastTime = now;

        glfwPollEvents();
        update(dt);

        int width = 0, height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        if (width == 0 || height == 0) {   // minimised: nothing to draw, so wait a little instead of spinning
            glfwWaitEventsTimeout(0.1);
        } else {
            render(width, height);
            glfwSwapBuffers(window_);
            ++fpsFrames;
        }

        if (now - fpsTime >= 1.0) {
            fps_ = fpsFrames;
            char title[64];
            std::snprintf(title, sizeof(title), "The Dark Maze - %d FPS", fps_);
            glfwSetWindowTitle(window_, title);
            fpsFrames = 0;
            fpsTime = now;
        }
        if (maxFrames > 0 && ++frames >= maxFrames) glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
    if (maxFrames > 0) std::printf("Average FPS: %.0f\n", frames / (glfwGetTime() - startTime));
    if (const GLenum error = glGetError(); error != GL_NO_ERROR) std::printf("OpenGL error 0x%04X\n", error);
}
