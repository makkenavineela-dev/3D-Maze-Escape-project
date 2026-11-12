#pragma once
#include <glm/vec2.hpp>
#include <random>
#include <vector>

// A wall piece in polar coordinates. Angles are in radians, measured from +X towards +Z.
// Arc walls follow a circle:  radiusInner == radiusOuter, angleStart < angleEnd.
// Radial walls run outwards:  angleStart == angleEnd, radiusInner < radiusOuter.
struct WallSegment {
    enum class Type { Arc, Radial };
    Type type;
    float radiusInner;
    float radiusOuter;
    float angleStart;
    float angleEnd;
};

struct CellId {
    int ring;
    int sector;
    bool operator==(const CellId& other) const { return ring == other.ring && sector == other.sector; }
};

// Circular maze on a polar grid (Person 2). Ring 0 is the round centre room; every other ring is
// split into sectors. It is carved with a recursive backtracker, so it is a perfect maze (every
// cell reachable, no loops), and a single exit is cut into the outer wall.
class Maze {
public:
    explicit Maze(unsigned seed);

    unsigned seed() const { return seed_; }
    int rings() const { return static_cast<int>(sectorCounts_.size()); }
    int sectors(int ring) const { return sectorCounts_[ring]; }
    int cellCount() const;
    float ringWidth() const;
    float outerRadius() const;
    int exitSector() const { return exitSector_; }   // sector of the outermost ring that has the exit

    std::vector<CellId> neighbours(CellId cell) const;   // adjacent cells, ignoring walls
    bool linked(CellId a, CellId b) const;                // true when there is no wall between a and b
    std::vector<WallSegment> walls() const;               // every wall, including the outer boundary
    CellId worldToCell(float x, float z) const;           // ring == rings() means outside the maze
    glm::vec2 cellCentre(CellId cell) const;              // world (x, z) of the middle of a cell
    std::vector<CellId> solve() const;                    // shortest path from the centre to the exit cell
    int reachableCells() const;                           // cells that can be reached from the centre

private:
    struct Cell {
        bool openInward = false;   // no wall towards the inner ring
        bool openNext = false;     // no wall towards sector + 1 in the same ring
    };

    void carve(std::mt19937& rng);
    void link(CellId a, CellId b);
    std::vector<std::vector<CellId>> bfsFromCentre() const;
    Cell& at(CellId c) { return cells_[c.ring][c.sector]; }
    const Cell& at(CellId c) const { return cells_[c.ring][c.sector]; }

    unsigned seed_;
    std::vector<int> sectorCounts_;
    std::vector<std::vector<Cell>> cells_;
    int exitSector_ = 0;
};
