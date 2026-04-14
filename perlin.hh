#pragma once

#include "image/image.hh"

PPM createRandomImage(int sx, int sy, int nb_octaves, float persistence = 0.5, float lacunarity = 2, int grid_size = 10);
