#pragma once

#include "image/image.hh"

PPM createRandomImage(int sx, int sy, int grid_size);
void procedural(int sx = 100, int sy = 100, int nb_images = 5, float weight = 0.5f, int grid_size = 8);
