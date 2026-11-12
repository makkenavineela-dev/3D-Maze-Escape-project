#pragma once
#include "Maze.h"
#include "Mesh.h"

// Procedural geometry for the maze (Person 3)
MeshData buildWallMesh(const std::vector<WallSegment>& walls);   // curved arc walls + straight radial walls
MeshData buildGroundMesh(float radius);                           // flat disc at y = 0
MeshData buildSkyMesh();                                          // unit sphere seen from the inside
