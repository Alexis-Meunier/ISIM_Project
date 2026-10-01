#pragma once

#include "image/image.hh"

float lerp(float a, float b, float x);

uint8_t computePerlinAtCoord3D(int x, int y, int z, int nb_octaves = 5, float persistence = 0.5, float lacunarity = 2, int grid_size = 10);