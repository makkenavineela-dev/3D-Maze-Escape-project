#include "MazeMesh.h"
#include "Config.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <utility>

namespace {

constexpr float kTexScale = 2.0f;                                  // metres covered by one texture repeat
constexpr float kMaxArcStep = 3.0f * glm::pi<float>() / 180.0f;    // curved walls get a new quad every 3 degrees
const glm::vec3 kUp(0.0f, 1.0f, 0.0f);

glm::vec3 radialDir(float angle) { return {std::cos(angle), 0.0f, std::sin(angle)}; }
glm::vec3 tangentDir(float angle) { return {-std::sin(angle), 0.0f, std::cos(angle)}; }   // direction of growing angle
glm::vec2 planarUV(const glm::vec3& p) { return {p.x / kTexScale, p.z / kTexScale}; }
Vertex vtx(const glm::vec3& p, const glm::vec3& n, const glm::vec2& uv) { return {p, n, uv}; }

// Curved wall: outer face, inner face and top, split into small quads, plus a cap at each end
void addArcWall(MeshData& mesh, const WallSegment& wall) {
    const float half = cfg::kWallThickness * 0.5f, h = cfg::kWallHeight, v = h / kTexScale;
    const float rIn = wall.radiusInner - half, rOut = wall.radiusInner + half;
    const float span = wall.angleEnd - wall.angleStart;
    const int steps = std::max(1, static_cast<int>(std::ceil(span / kMaxArcStep)));

    for (int i = 0; i < steps; ++i) {
        const float a0 = wall.angleStart + span * i / steps;
        const float a1 = wall.angleStart + span * (i + 1) / steps;
        const glm::vec3 d0 = radialDir(a0), d1 = radialDir(a1);

        // side faces: normals point away from the wall's centre line, so lighting follows the curve
        for (const auto& [r, side] : {std::pair{rOut, 1.0f}, std::pair{rIn, -1.0f}}) {
            const float u0 = a0 * r / kTexScale, u1 = a1 * r / kTexScale;
            mesh.addQuad(vtx(d0 * r, d0 * side, {u0, 0.0f}), vtx(d1 * r, d1 * side, {u1, 0.0f}),
                         vtx(d1 * r + kUp * h, d1 * side, {u1, v}), vtx(d0 * r + kUp * h, d0 * side, {u0, v}));
        }
        const glm::vec3 p0 = d0 * rIn + kUp * h, p1 = d0 * rOut + kUp * h;
        const glm::vec3 p2 = d1 * rOut + kUp * h, p3 = d1 * rIn + kUp * h;
        mesh.addQuad(vtx(p0, kUp, planarUV(p0)), vtx(p1, kUp, planarUV(p1)),
                     vtx(p2, kUp, planarUV(p2)), vtx(p3, kUp, planarUV(p3)));
    }

    for (const auto& [angle, side] : {std::pair{wall.angleStart, -1.0f}, std::pair{wall.angleEnd, 1.0f}}) {
        const glm::vec3 d = radialDir(angle), n = tangentDir(angle) * side;
        const float u = cfg::kWallThickness / kTexScale;
        mesh.addQuad(vtx(d * rIn, n, {0.0f, 0.0f}), vtx(d * rOut, n, {u, 0.0f}),
                     vtx(d * rOut + kUp * h, n, {u, v}), vtx(d * rIn + kUp * h, n, {0.0f, v}));
    }
}

// Straight wall pointing away from the centre: two long sides, top and two end caps
void addRadialWall(MeshData& mesh, const WallSegment& wall) {
    const float half = cfg::kWallThickness * 0.5f, h = cfg::kWallHeight, v = h / kTexScale;
    // reach half a wall thickness into the curved walls at both ends so corners have no gaps
    const float r0 = std::max(0.0f, wall.radiusInner - half), r1 = wall.radiusOuter + half;
    const glm::vec3 d = radialDir(wall.angleStart), t = tangentDir(wall.angleStart);

    for (float side : {1.0f, -1.0f}) {
        const glm::vec3 offset = t * (half * side), n = t * side;
        mesh.addQuad(vtx(d * r0 + offset, n, {r0 / kTexScale, 0.0f}), vtx(d * r1 + offset, n, {r1 / kTexScale, 0.0f}),
                     vtx(d * r1 + offset + kUp * h, n, {r1 / kTexScale, v}),
                     vtx(d * r0 + offset + kUp * h, n, {r0 / kTexScale, v}));
    }
    const glm::vec3 p0 = d * r0 - t * half + kUp * h, p1 = d * r0 + t * half + kUp * h;
    const glm::vec3 p2 = d * r1 + t * half + kUp * h, p3 = d * r1 - t * half + kUp * h;
    mesh.addQuad(vtx(p0, kUp, planarUV(p0)), vtx(p1, kUp, planarUV(p1)),
                 vtx(p2, kUp, planarUV(p2)), vtx(p3, kUp, planarUV(p3)));

    for (const auto& [r, side] : {std::pair{r0, -1.0f}, std::pair{r1, 1.0f}}) {
        const glm::vec3 base = d * r, n = d * side;
        const float u = cfg::kWallThickness / kTexScale;
        mesh.addQuad(vtx(base - t * half, n, {0.0f, 0.0f}), vtx(base + t * half, n, {u, 0.0f}),
                     vtx(base + t * half + kUp * h, n, {u, v}), vtx(base - t * half + kUp * h, n, {0.0f, v}));
    }
}

}  // namespace

MeshData buildWallMesh(const std::vector<WallSegment>& walls) {
    MeshData mesh;
    for (const WallSegment& wall : walls) {
        if (wall.type == WallSegment::Type::Arc) addArcWall(mesh, wall);
        else addRadialWall(mesh, wall);
    }
    return mesh;
}

MeshData buildGroundMesh(float radius) {
    MeshData mesh;
    constexpr int kSegments = 96;
    const Vertex centre = vtx(glm::vec3(0.0f), kUp, {0.0f, 0.0f});
    for (int i = 0; i < kSegments; ++i) {
        const glm::vec3 p0 = radialDir(glm::two_pi<float>() * i / kSegments) * radius;
        const glm::vec3 p1 = radialDir(glm::two_pi<float>() * (i + 1) / kSegments) * radius;
        mesh.addTriangle(centre, vtx(p0, kUp, planarUV(p0)), vtx(p1, kUp, planarUV(p1)));
    }
    return mesh;
}

MeshData buildExitTriggerMesh(const Maze& maze) {
    MeshData mesh;
    const glm::vec2 exit = maze.exitWorldPos();
    const float centreAngle = std::atan2(exit.y, exit.x);
    const float innerRadius = maze.outerRadius() + cfg::kExitTriggerOffset - cfg::kExitTriggerThickness * 0.5f;
    const float outerRadius = maze.outerRadius() + cfg::kExitTriggerOffset + cfg::kExitTriggerThickness * 0.5f;
    const float startAngle = centreAngle - cfg::kExitTriggerHalfAngle;
    const float endAngle = centreAngle + cfg::kExitTriggerHalfAngle;
    const int steps = std::max(1, static_cast<int>(std::ceil((endAngle - startAngle) / kMaxArcStep)));
    constexpr float y = 0.02f;  // lift slightly above the ground to avoid z-fighting

    for (int i = 0; i < steps; ++i) {
        const float a0 = startAngle + (endAngle - startAngle) * i / steps;
        const float a1 = startAngle + (endAngle - startAngle) * (i + 1) / steps;
        const glm::vec3 inner0(std::cos(a0) * innerRadius, y, std::sin(a0) * innerRadius);
        const glm::vec3 inner1(std::cos(a1) * innerRadius, y, std::sin(a1) * innerRadius);
        const glm::vec3 outer0(std::cos(a0) * outerRadius, y, std::sin(a0) * outerRadius);
        const glm::vec3 outer1(std::cos(a1) * outerRadius, y, std::sin(a1) * outerRadius);
        mesh.addQuad(vtx(inner0, kUp, planarUV(inner0)), vtx(outer0, kUp, planarUV(outer0)),
                     vtx(outer1, kUp, planarUV(outer1)), vtx(inner1, kUp, planarUV(inner1)));
    }
    return mesh;
}

MeshData buildSkyMesh() {
    MeshData mesh;
    constexpr int kStacks = 16, kSlices = 32;
    auto point = [](float pitch, float yaw) {
        return glm::vec3(std::cos(pitch) * std::cos(yaw), std::sin(pitch), std::cos(pitch) * std::sin(yaw));
    };
    for (int i = 0; i < kStacks; ++i) {
        const float p0 = -glm::half_pi<float>() + glm::pi<float>() * i / kStacks;
        const float p1 = -glm::half_pi<float>() + glm::pi<float>() * (i + 1) / kStacks;
        for (int j = 0; j < kSlices; ++j) {
            const float y0 = glm::two_pi<float>() * j / kSlices, y1 = glm::two_pi<float>() * (j + 1) / kSlices;
            const glm::vec3 q0 = point(p0, y0), q1 = point(p0, y1), q2 = point(p1, y1), q3 = point(p1, y0);
            // normals point inwards so the inside of the sphere faces the camera
            mesh.addQuad(vtx(q0, -q0, {}), vtx(q1, -q1, {}), vtx(q2, -q2, {}), vtx(q3, -q3, {}));
        }
    }
    return mesh;
}
