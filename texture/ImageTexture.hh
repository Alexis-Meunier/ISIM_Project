#pragma once

#include "texture.hh"
#include <vector>
#include <string>

class ImageTexture : public TextureMaterial
{
public:
    ImageTexture() = default;
    ImageTexture(const std::string& filepath, float kd = 0.8f, float ks = 0.2f,
                 float ka = 0.1f, float ns = 32.0f);
    ~ImageTexture();

    TextureInfo* get_elements(Point4 position) override;

    TextureInfo* get_elements_sphere(Point4 hit_point, Point4 center, float radius);

    TextureInfo* get_elements_uv(float u, float v);

private:
    std::vector<unsigned char> pixels;
    int width = 0, height = 0, channels = 0;

    float kd, ks, ka, ns;
    TextureInfo* cached_info = nullptr;

    Color sample(float u, float v);
};