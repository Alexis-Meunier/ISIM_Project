#pragma once

#include "utils/color.hh"
#include "utils/point4.hh"
#include "utils/vector4.hh"

struct TextureInfo
{
    float kd; // diffuse part
    float ks; // specular part
    bool kr; // is glass ?
    float eta = 1.0; // refraction index (air = 1, glass = 1.5, diamond = 2.4)
    Color* color = NULL; // Color of the light
    float lightPower = 1.0; // Power of light emission
};

class TextureMaterial
{
public:
    TextureMaterial() = default;

    virtual TextureInfo *get_elements(Point4 position) = 0;
    bool isLight = false;
};
