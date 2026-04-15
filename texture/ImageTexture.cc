#include "ImageTexture.hh"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#include <cmath>
#include <stdexcept>


ImageTexture::ImageTexture(const std::string& filepath, bool isLight, float kd, float ks,
                           float ka, float ns)
    : kd(kd), ks(ks), ka(ka), ns(ns)
{
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
    if (!data)
        throw std::runtime_error("ImageTexture: failed to load " + filepath);

    channels = 3;
    // TODO: Handle transparent pixels
    pixels.assign(data, data + width * height * 3);
    stbi_image_free(data);

    cached_info = new TextureInfo();
    cached_info->kd = kd;
    cached_info->ks = ks;
    cached_info->lightPower = 0.0f;
    cached_info->color = new Color();
    this->isLight = isLight;
}

ImageTexture::~ImageTexture()
{
    if (cached_info) {
        delete cached_info->color;
        delete cached_info;
    }
}

Color ImageTexture::sample(float u, float v)
{
    u = std::fmax(0.0f, std::fmin(1.0f, u));
    v = std::fmax(0.0f, std::fmin(1.0f, v));

    v = 1.0f - v;

    int x = (int)(u * (width  - 1));
    int y = (int)(v * (height - 1));

    x = std::max(0, std::min(width  - 1, x));
    y = std::max(0, std::min(height - 1, y));

    int idx = (y * width + x) * 3;
    float r = pixels[idx];
    float g = pixels[idx + 1];
    float b = pixels[idx + 2];

    return Color(r, g, b);
}

TextureInfo* ImageTexture::get_elements(Point4 position)
{
    return get_elements_uv(0.5f, 0.5f);
}

TextureInfo* ImageTexture::get_elements_sphere(Point4 hit_point, Point4 center, float radius)
{
    float nx = (hit_point.x - center.x) / radius;
    float nz = (hit_point.z - center.z) / radius;

    float phi = std::atan2(nz, nx);;
    float theta = std::acos(nz);

    float u = (phi + M_PI) / (2.0f * M_PI);
    float v = theta / M_PI;

    return get_elements_uv(u, v);
}

TextureInfo* ImageTexture::get_elements_uv(float u, float v)
{
    *cached_info->color = sample(u, v);
    return cached_info;
}