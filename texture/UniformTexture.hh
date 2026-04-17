#pragma once

#include "texture.hh"

class UniformTexture : public TextureMaterial
{
public:
    UniformTexture();
    UniformTexture(TextureInfo* info);
    UniformTexture(const UniformTexture& info);
    UniformTexture(const Color& info);
    UniformTexture(const Color& color, float kd, float ks, bool kr, float eta, bool isLight = false);
    UniformTexture(const Color& color, float kd, float ks, bool isLight = false);

    TextureInfo* get_elements(Point4 position) override;

private:
    TextureInfo* info;
};
