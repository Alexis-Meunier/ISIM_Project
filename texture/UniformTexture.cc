#include "UniformTexture.hh"

TextureInfo* UniformTexture::get_elements(Point4 position)
{
    return info;
}

UniformTexture::UniformTexture()
{
    TextureInfo* textInfo = new TextureInfo();
    textInfo->kd = 1;
    textInfo->ks = 0;
    textInfo->kr = false;
    textInfo->eta = 0;
    textInfo->color = new Color();
    info = textInfo;
    textInfo->lightPower = 0;
    isLight = false;
}

UniformTexture::UniformTexture(const Color& color, bool isLight)
{
    TextureInfo* textInfo = new TextureInfo();
    textInfo->kd = 1;
    textInfo->ks = 0;
    textInfo->kr = false;
    textInfo->eta = 0;
    textInfo->color = new Color(color);
    info = textInfo;
    textInfo->lightPower = 0;
    this->isLight = isLight;
}

UniformTexture::UniformTexture(const Color& color, float kd, float ks, bool isLight)
{
    TextureInfo* textInfo = new TextureInfo();
    textInfo->kd = kd;
    textInfo->ks = ks;
    textInfo->kr = false;
    textInfo->eta = 0;
    textInfo->color = new Color(color);
    info = textInfo;
    textInfo->lightPower = 0;
    this->isLight = isLight;
}

UniformTexture::UniformTexture(const Color& color, float kd, float ks, bool kr,
                               float eta, bool isLight)
{
    TextureInfo* textInfo = new TextureInfo();
    textInfo->kd = kd;
    textInfo->ks = ks;
    textInfo->kr = kr;
    textInfo->eta = eta;
    textInfo->color = new Color(color);
    info = textInfo;
    textInfo->lightPower = 0;
    this->isLight = isLight;
}

UniformTexture::UniformTexture(TextureInfo* info, bool isLight)
{
    this->info = info;
    this->isLight = isLight;
}

UniformTexture::UniformTexture(const UniformTexture& info, bool isLight)
{
    this->info = info.info;
    this->isLight = isLight;
}
