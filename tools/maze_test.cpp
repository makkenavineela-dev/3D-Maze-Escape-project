// Maze checks (Person 2): generates many mazes and verifies that each one is perfect and solvable.
//
//   maze_test                       test seeds 1..1000
//   maze_test 5000                  test seeds 1..5000
//   maze_test --svg maze.svg 42     draw maze 42 from above as an SVG picture (open it in a browser)
#include "Maze.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <glm/gtc/constants.hpp>
#include <string>
#include <utility>

namespace {

int countLinks(const Maze& maze) {
    int links = 0;
    for (int r = 0; r < maze.rings(); ++r)
        for (int s = 0; s < maze.sectors(r); ++s)
            for (CellId n : maze.neighbours({r, s}))
                if (std::make_pair(n.ring, n.sector) > std::make_pair(r, s) && maze.linked({r, s}, n)) ++links;
    return links;
}

int countDeadEnds(const Maze& maze) {
    int deadEnds = 0;
    for (int r = 0; r < maze.rings(); ++r)
        for (int s = 0; s < maze.sectors(r); ++s) {
            int open = 0;
            for (CellId n : maze.neighbours({r, s}))
                if (maze.linked({r, s}, n)) ++open;
            if (open == 1) ++deadEnds;
        }
    return deadEnds;
}

int countOuterWalls(const Maze& maze) {
    int count = 0;
    for (const WallSegment& w : maze.walls())
        if (w.type == WallSegment::Type::Arc && w.radiusInner == maze.outerRadius()) ++count;
    return count;
}

// Returns an empty string when the maze passes, otherwise what is wrong with it
std::string check(const Maze& maze) {
    const int cells = maze.cellCount();
    if (maze.reachableCells() != cells) return "not every cell can be reached";
    if (countLinks(maze) != cells - 1) return "maze has loops (not a perfect maze)";
    if (countOuterWalls(maze) != maze.sectors(maze.rings() - 1) - 1) return "outer wall does not have exactly one exit";
    const auto path = maze.solve();
    if (path.empty()) return "exit cannot be reached";
    if (!(path.front() == CellId{0, 0})) return "path does not start at the centre";
    if (!(path.back() == CellId{maze.rings() - 1, maze.exitSector()})) return "path does not end at the exit";
    for (size_t i = 1; i < path.size(); ++i)
        if (!maze.linked(path[i - 1], path[i])) return "path goes through a wall";
    return "";
}

void writeSvg(const Maze& maze, const std::string& fileName) {
    const float scale = 12.0f, margin = 30.0f;
    const float size = 2.0f * (maze.outerRadius() * scale + margin);
    const float c = size / 2.0f;
    auto px = [&](float x) { return c + x * scale; };

    std::ofstream svg(fileName);
    svg << "<svg xmlns='http://www.w3.org/2000/svg' width='" << size << "' height='" << size << "'>\n"
        << "<rect width='100%' height='100%' fill='#101418'/>\n"
        << "<g stroke='#d8d2c4' stroke-width='3' stroke-linecap='round' fill='none'>\n";
    for (const WallSegment& w : maze.walls()) {
        const float r = w.radiusInner;
        if (w.type == WallSegment::Type::Arc) {
            const int largeArc = (w.angleEnd - w.angleStart) > glm::pi<float>() ? 1 : 0;
            svg << "<path d='M " << px(r * std::cos(w.angleStart)) << ' ' << px(r * std::sin(w.angleStart))
                << " A " << r * scale << ' ' << r * scale << " 0 " << largeArc << " 1 "
                << px(r * std::cos(w.angleEnd)) << ' ' << px(r * std::sin(w.angleEnd)) << "'/>\n";
        } else {
            const float a = w.angleStart;
            svg << "<line x1='" << px(w.radiusInner * std::cos(a)) << "' y1='" << px(w.radiusInner * std::sin(a))
                << "' x2='" << px(w.radiusOuter * std::cos(a)) << "' y2='" << px(w.radiusOuter * std::sin(a)) << "'/>\n";
        }
    }
    svg << "</g>\n<polyline fill='none' stroke='#e5484d' stroke-width='2' stroke-dasharray='6 4' points='";
    for (CellId cell : maze.solve()) {
        const glm::vec2 p = maze.cellCentre(cell);
        svg << px(p.x) << ',' << px(p.y) << ' ';
    }
    const float exitAngle = (maze.exitSector() + 0.5f) * glm::two_pi<float>() / maze.sectors(maze.rings() - 1);
    const float outside = maze.outerRadius() + 1.5f;
    svg << px(outside * std::cos(exitAngle)) << ',' << px(outside * std::sin(exitAngle)) << "'/>\n"
        << "<circle cx='" << c << "' cy='" << c << "' r='6' fill='#46a758'/>\n"
        << "<text x='12' y='24' fill='#d8d2c4' font-family='sans-serif' font-size='16'>Seed " << maze.seed()
        << " - green = start, red = solution</text>\n</svg>\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc >= 3 && std::strcmp(argv[1], "--svg") == 0) {
        const unsigned seed = argc >= 4 ? static_cast<unsigned>(std::strtoul(argv[3], nullptr, 10)) : 1u;
        Maze maze(seed);
        writeSvg(maze, argv[2]);
        std::printf("Wrote %s (seed %u, %d cells, solution %zu cells long)\n", argv[2], seed, maze.cellCount(),
                    maze.solve().size());
        return 0;
    }

    const int count = argc >= 2 ? std::atoi(argv[1]) : 1000;
    Maze first(1);
    std::printf("Polar grid: %d rings, sectors per ring:", first.rings());
    for (int r = 0; r < first.rings(); ++r) std::printf(" %d", first.sectors(r));
    std::printf(" (%d cells)\n", first.cellCount());

    int failures = 0, shortest = 1 << 30, longest = 0;
    long long totalLength = 0, totalDeadEnds = 0;
    for (int seed = 1; seed <= count; ++seed) {
        Maze maze(static_cast<unsigned>(seed));
        const std::string problem = check(maze);
        if (!problem.empty()) {
            if (++failures <= 10) std::printf("FAIL seed %d: %s\n", seed, problem.c_str());
            continue;
        }
        const int length = static_cast<int>(maze.solve().size());
        shortest = std::min(shortest, length);
        longest = std::max(longest, length);
        totalLength += length;
        totalDeadEnds += countDeadEnds(maze);
    }

    const int passed = count - failures;
    std::printf("Tested %d mazes: %d passed, %d failed\n", count, passed, failures);
    if (passed > 0)
        std::printf("Solution length (cells): shortest %d, average %.1f, longest %d; average dead ends %.1f\n",
                    shortest, double(totalLength) / passed, longest, double(totalDeadEnds) / passed);
    return failures == 0 ? 0 : 1;
}
