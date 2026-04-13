#pragma once

#include <vector>

#include "texture.hh"
#include "../utils/point4.hh"

class ProceduralTexture : public TextureMaterial
{
public:
    TextureInfo *get_elements(Point4 position) override;

    float weight;
    int nb_points;

private:
    std::vector<Point4> generated_points;
};
