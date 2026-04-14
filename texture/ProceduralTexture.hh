#pragma once

#include <vector>

#include "texture.hh"
#include "../utils/point4.hh"
#include "../perlin.hh"

enum ProceduralType
{
    CLOUD = 0,
    WOOD,
    NOISE,
};

class ProceduralTexture : public TextureMaterial
{
public:
    ProceduralTexture(ProceduralType type, float kd = 0.8f, float ks = 0.2f,
                 float ka = 0.1f, float ns = 32.0f);
    TextureInfo *get_elements(Point4 position) override;

    ProceduralType type;
    TextureInfo *info;
};
