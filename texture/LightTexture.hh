#pragma once

#include "texture.hh"

struct LightInfo: public TextureInfo
{
};

class LightTexture: public TextureMaterial
{
public:
    LightTexture();
    LightTexture(TextureInfo *info);
    LightTexture(const LightTexture& info);
    LightTexture(const Color& info);

    TextureInfo *get_elements(Point4 position) override;

private:
    TextureInfo *info;
};
