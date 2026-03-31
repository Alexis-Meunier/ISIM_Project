#include "UniformTexture.hh"

TextureInfo UniformTexture::get_elements(Point4 position)
{
    return info;
}

UniformTexture::UniformTexture()
{
    TextureInfo textInfo;
    textInfo.kd = 1;
    textInfo.ks = 0.2;
    textInfo.ka = 0.5;
    textInfo.ns = 0.8;
    textInfo.color = new Color();
    info = textInfo;
}

UniformTexture::UniformTexture(const Color& color)
{
    TextureInfo textInfo;
    textInfo.kd = 1;
    textInfo.ks = 0.2;
    textInfo.ka = 0.5;
    textInfo.ns = 0.8;
    textInfo.color = new Color(color);
    info = textInfo;
}

UniformTexture::UniformTexture(const TextureInfo& info)
{
    this->info = info;
}

UniformTexture::UniformTexture(const UniformTexture& info)
{
    this->info = info.info;
}
