#include "LightTexture.hh"

TextureInfo *LightTexture::get_elements(Point4 position)
{
    return info;
}

LightTexture::LightTexture()
{
    TextureInfo *textInfo = new TextureInfo();
    textInfo->kd = 1;
    textInfo->ks = 0.2;
    textInfo->ka = 0.5;
    textInfo->ns = 0.8;
    textInfo->color = new Color();
    info = textInfo;
    textInfo->lightPower = 0;
}

LightTexture::LightTexture(const Color& color)
{
    TextureInfo *textInfo = new TextureInfo();
    textInfo->kd = 1;
    textInfo->ks = 0.2;
    textInfo->ka = 0.5;
    textInfo->ns = 0.8;
    textInfo->color = new Color(color);
    info = textInfo;
    textInfo->lightPower = 0;
}

LightTexture::LightTexture(TextureInfo *info)
{
    this->info = info;
}

LightTexture::LightTexture(const LightTexture& info)
{
    this->info = info.info;
}
