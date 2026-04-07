#pragma once

#include "texture.hh"

class UniformTexture : public TextureMaterial
{
public:
    UniformTexture();
    UniformTexture(TextureInfo* info);
    UniformTexture(const UniformTexture& info);
    UniformTexture(const Color& info);
    UniformTexture(const Color& color, float kd, float ks, float kr, float eta);
    UniformTexture(const Color& color, float kd, float ks);

    TextureInfo* get_elements(Point4 position) override;
};
