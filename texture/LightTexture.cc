#include "LightTexture.hh"

TextureInfo* LightTexture::get_elements(Point4 position)
{
    return info;
}

LightTexture::LightTexture()
{
    TextureInfo* textInfo = new TextureInfo();
    textInfo->kd = 1;
    textInfo->ks = 0;
    textInfo->kr = 0;
    textInfo->eta = 0;
    textInfo->color = new Color();
    info = textInfo;
    textInfo->lightPower = 0;
}

LightTexture::LightTexture(const Color& color)
{
    TextureInfo* textInfo = new TextureInfo();
    textInfo->kd = 1;
    textInfo->ks = 0;
    textInfo->kr = 0;
    textInfo->eta = 0;
    textInfo->color = new Color(color);
    info = textInfo;
    textInfo->lightPower = 0;
}

LightTexture::LightTexture(TextureInfo* info)
{
    this->info = info;
}

LightTexture::LightTexture(const LightTexture& info)
{
    this->info = info.info;
}
