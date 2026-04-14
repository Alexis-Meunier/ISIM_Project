#pragma once

#include "image/image.hh"

float lerp(float a, float b, float x);

uint8_t computePerlinAtCoord(int x, int y, int nb_octaves = 5, float persistence = 0.5, float lacunarity = 2, int grid_size = 10);
PPM createRandomImage(int sx, int sy, int nb_octaves = 5, float persistence = 0.5, float lacunarity = 2, int grid_size = 10);
PPM createWoodTexture(int sx, int sy, int nb_octaves = 6, float persistence = 0.7, float lacunarity = 2, int grid_size = 120);
PPM createCloudTexture(int sx, int sy, int nb_octaves = 4, float persistence = 0.6, float lacunarity = 2, int grid_size = 200);
