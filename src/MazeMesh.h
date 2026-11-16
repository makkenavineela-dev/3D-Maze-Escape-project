#pragma once
#include "Maze.h"
#include "Mesh.h"

// Procedural geometry for the maze
MeshData buildWallMesh(const std::vector<WallSegment>& walls);   // curved arc walls + straight radial walls
MeshData buildGroundMesh(float radius);                           // flat disc at y = 0
MeshData buildExitTriggerMesh(const Maze& maze);                    // highlighted arc matching the exit trigger
MeshData buildSkyMesh();                                          // unit sphere seen from the inside
