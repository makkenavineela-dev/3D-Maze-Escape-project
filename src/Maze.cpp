#include "Maze.h"
#include "Config.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <queue>

namespace {
constexpr float kTwoPi = glm::two_pi<float>();
}

Maze::Maze(unsigned seed) : seed_(seed) {
    // Sectors per ring: ring 1 starts with kInnerSectors, and a ring doubles the count of the ring
    // inside it whenever its cells would otherwise get too wide
    sectorCounts_.push_back(1);
    for (int r = 1; r < cfg::kRings; ++r) {
        int count = (r == 1) ? cfg::kInnerSectors : sectorCounts_.back();
        const float midRadius = (r + 0.5f) * cfg::kRingWidth;
        while (kTwoPi * midRadius / count > cfg::kMaxCellWidthRatio * cfg::kRingWidth) count *= 2;
        sectorCounts_.push_back(count);
    }
    cells_.resize(sectorCounts_.size());
    for (size_t r = 0; r < cells_.size(); ++r) cells_[r].resize(sectorCounts_[r]);

    std::mt19937 rng(seed);
    carve(rng);
    std::uniform_int_distribution<int> pickExit(0, sectorCounts_.back() - 1);
    exitSector_ = pickExit(rng);
}

int Maze::cellCount() const {
    int total = 0;
    for (int count : sectorCounts_) total += count;
    return total;
}

float Maze::ringWidth() const { return cfg::kRingWidth; }

float Maze::outerRadius() const { return rings() * cfg::kRingWidth; }

// Recursive backtracker (depth-first search) with an explicit stack instead of recursion
void Maze::carve(std::mt19937& rng) {
    std::vector<std::vector<bool>> visited(cells_.size());
    for (size_t r = 0; r < cells_.size(); ++r) visited[r].assign(cells_[r].size(), false);

    std::vector<CellId> stack{{0, 0}};
    visited[0][0] = true;
    while (!stack.empty()) {
        const CellId current = stack.back();
        std::vector<CellId> options;
        for (CellId next : neighbours(current))
            if (!visited[next.ring][next.sector]) options.push_back(next);

        if (options.empty()) {   // dead end: step back
            stack.pop_back();
            continue;
        }
        std::uniform_int_distribution<size_t> pick(0, options.size() - 1);
        const CellId next = options[pick(rng)];
        link(current, next);
        visited[next.ring][next.sector] = true;
        stack.push_back(next);
    }
}

std::vector<CellId> Maze::neighbours(CellId cell) const {
    std::vector<CellId> result;
    const int count = sectorCounts_[cell.ring];
    if (cell.ring > 0) {
        result.push_back({cell.ring, (cell.sector + 1) % count});
        result.push_back({cell.ring, (cell.sector + count - 1) % count});
        // the cell of the inner ring that covers this cell's angle
        result.push_back({cell.ring - 1, cell.sector * sectorCounts_[cell.ring - 1] / count});
    }
    if (cell.ring + 1 < rings()) {
        const int ratio = sectorCounts_[cell.ring + 1] / count;
        for (int i = 0; i < ratio; ++i) result.push_back({cell.ring + 1, cell.sector * ratio + i});
    }
    return result;
}

void Maze::link(CellId a, CellId b) {
    if (a.ring == b.ring) {
        const int count = sectorCounts_[a.ring];
        at((a.sector + 1) % count == b.sector ? a : b).openNext = true;
    } else {
        at(a.ring > b.ring ? a : b).openInward = true;
    }
}

bool Maze::linked(CellId a, CellId b) const {
    if (a.ring == b.ring) {
        if (a.ring == 0) return false;
        const int count = sectorCounts_[a.ring];
        if ((a.sector + 1) % count == b.sector) return at(a).openNext;
        if ((b.sector + 1) % count == a.sector) return at(b).openNext;
        return false;
    }
    const CellId& inner = a.ring < b.ring ? a : b;
    const CellId& outer = a.ring < b.ring ? b : a;
    if (outer.ring != inner.ring + 1) return false;
    const int parent = outer.sector * sectorCounts_[inner.ring] / sectorCounts_[outer.ring];
    return parent == inner.sector && at(outer).openInward;
}

std::vector<WallSegment> Maze::walls() const {
    std::vector<WallSegment> result;
    const float w = cfg::kRingWidth;
    for (int r = 1; r < rings(); ++r) {
        const float step = kTwoPi / sectorCounts_[r];
        for (int s = 0; s < sectorCounts_[r]; ++s) {
            const Cell& cell = cells_[r][s];
            const float a0 = s * step, a1 = (s + 1) * step;
            if (!cell.openInward) result.push_back({WallSegment::Type::Arc, r * w, r * w, a0, a1});
            if (!cell.openNext) result.push_back({WallSegment::Type::Radial, r * w, (r + 1) * w, a1, a1});
        }
    }
    // Outer boundary: closed everywhere except the exit sector
    const int outer = rings() - 1;
    const float step = kTwoPi / sectorCounts_[outer];
    const float radius = outerRadius();
    for (int s = 0; s < sectorCounts_[outer]; ++s)
        if (s != exitSector_) result.push_back({WallSegment::Type::Arc, radius, radius, s * step, (s + 1) * step});
    return result;
}

CellId Maze::worldToCell(float x, float z) const {
    const float radius = std::sqrt(x * x + z * z);
    const int ring = static_cast<int>(radius / cfg::kRingWidth);
    if (ring >= rings()) return {rings(), 0};
    if (ring == 0) return {0, 0};
    float angle = std::atan2(z, x);
    if (angle < 0.0f) angle += kTwoPi;
    const int count = sectorCounts_[ring];
    return {ring, std::min(static_cast<int>(angle / kTwoPi * count), count - 1)};
}

glm::vec2 Maze::cellCentre(CellId cell) const {
    if (cell.ring == 0) return {0.0f, 0.0f};
    const float radius = (cell.ring + 0.5f) * cfg::kRingWidth;
    const float angle = (cell.sector + 0.5f) * kTwoPi / sectorCounts_[cell.ring];
    return {radius * std::cos(angle), radius * std::sin(angle)};
}

// Breadth-first search from the centre; from[r][s] is the cell we came from ({-1, -1} = not reached)
std::vector<std::vector<CellId>> Maze::bfsFromCentre() const {
    std::vector<std::vector<CellId>> from(cells_.size());
    for (size_t r = 0; r < cells_.size(); ++r) from[r].assign(cells_[r].size(), CellId{-1, -1});

    std::queue<CellId> queue;
    from[0][0] = CellId{0, 0};
    queue.push({0, 0});
    while (!queue.empty()) {
        const CellId cell = queue.front();
        queue.pop();
        for (CellId next : neighbours(cell)) {
            if (from[next.ring][next.sector].ring < 0 && linked(cell, next)) {
                from[next.ring][next.sector] = cell;
                queue.push(next);
            }
        }
    }
    return from;
}

std::vector<CellId> Maze::solve() const {
    const auto from = bfsFromCentre();
    CellId cell{rings() - 1, exitSector_};
    if (from[cell.ring][cell.sector].ring < 0) return {};

    std::vector<CellId> path{cell};
    while (!(cell == CellId{0, 0})) {
        cell = from[cell.ring][cell.sector];
        path.push_back(cell);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

int Maze::reachableCells() const {
    int count = 0;
    for (const auto& ring : bfsFromCentre())
        for (const CellId& cell : ring)
            if (cell.ring >= 0) ++count;
    return count;
}
