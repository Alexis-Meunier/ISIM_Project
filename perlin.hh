#pragma once

#include "image/image.hh"


uint8_t computePerlinAtCoord(int x, int y, int nb_octaves = 5, float persistence = 0.5, float lacunarity = 2, int grid_size = 10);
PPM createRandomImage(int sx, int sy, int nb_octaves = 5, float persistence = 0.5, float lacunarity = 2, int grid_size = 10);
PPM createWoodTexture(int sx, int sy, int nb_octaves = 5, float persistence = 0.2, float lacunarity = 2, int grid_size = 50);
PPM createCloudTexture(int sx, int sy, int nb_octaves = 5, float persistence = 0.5, float lacunarity = 2, int grid_size = 10);
