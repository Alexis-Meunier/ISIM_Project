#pragma once

#include <vector>

#include "texture.hh"
#include "utils/point4.hh"
#include "perlin3D.hh"

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
                 float lightPower = 0.8f, bool isLight = false);
    TextureInfo *get_elements(Point4 position) override;

    ProceduralType type;
    TextureInfo *info;
};
