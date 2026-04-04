#pragma once

#include "../utils/color.hh"
#include "../utils/point4.hh"
#include "../utils/vector4.hh"

struct TextureInfo
{
    float kd; // diffuse reflection
    float ks; // specular reflection
    float ka; // ambient reflection
    float ns; // shininess constant
    Color* color; // Color of the light
    float lightPower; // Power of light emission
};

class TextureMaterial
{
public:
    TextureMaterial() = default;

    virtual TextureInfo *get_elements(Point4 position) = 0;

    TextureInfo *info;
};
