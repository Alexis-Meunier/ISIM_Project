#pragma once

#include "../utils/vector4.hh"
#include "../utils/point4.hh"
#include "../utils/color.hh"

struct TextureInfo
{
    float kd; // diffuse reflection
    float ks; // specular reflection
    float ka; // ambient reflection 
    float ns; // shininess constant
    Color *color; // Color of the light
};

class TextureMaterial
{
public:
    TextureMaterial() = default;

    virtual TextureInfo get_elements(Point4 position) = 0;

    TextureInfo info;
};

// class TransparentTexture : public TextureMaterial
// {
// public:
//     TransparentTexture();
//     TransparentTexture(const float& refraction);
//     TransparentTexture(const TextureInfo& info, const float& refraction);
//     TransparentTexture(const Pixel& info, const float& refraction);

//     TextureInfo get_elements(Point4 position) override;
//     float refraction_index;
// };
