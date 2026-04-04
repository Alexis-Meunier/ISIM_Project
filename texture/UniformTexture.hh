#pragma once

#include "texture.hh"

class UniformTexture : public TextureMaterial
{
public:
    UniformTexture();
    UniformTexture(TextureInfo *info);
    UniformTexture(const UniformTexture& info);
    UniformTexture(const Color& info);

    TextureInfo *get_elements(Point4 position) override;
};
