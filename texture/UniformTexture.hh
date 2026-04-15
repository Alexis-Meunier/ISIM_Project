#pragma once

#include "texture.hh"

class UniformTexture : public TextureMaterial
{
public:
    UniformTexture();
    UniformTexture(TextureInfo* info, bool isLight = false);
    UniformTexture(const UniformTexture& info, bool isLight = false);
    UniformTexture(const Color& info, bool isLight = false);
    UniformTexture(const Color& color, float kd, float ks, float kr, float eta, bool isLight = false);
    UniformTexture(const Color& color, float kd, float ks, bool isLight = false);

    TextureInfo *get_elements(Point4 position) override;

private:
    TextureInfo *info;
};
