#include "perlin.hh"

#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <vector>

// Compute Random gradient at grid corners
std::pair<float, float> gradient(int ix, int iy, int seed)
{
    uint32_t h = (uint32_t)ix * 1619u + (uint32_t)iy * 31337u + (uint32_t)seed * 6971u;
    h ^= (h >> 13);
    h *= 1234577u;
    h ^= (h >> 15);
    float angle = (h & 0xFFFFu) / 65536.f * 2.f * M_PI;
    return { std::cos(angle), std::sin(angle) };
}

// Linear interpolation
float lerp(float a, float b, float x)
{
    return a + x * (b - a);
}

// smootherStep
float smootherStep(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float perlin(float x, float y, int grid_size, int seed)
{
    int x0 = (int)std::floor(x / grid_size);
    int y0 = (int)std::floor(y / grid_size);

    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float dx = (x - x0 * grid_size) / grid_size;
    float dy = (y - y0 * grid_size) / grid_size;

    auto g00 = gradient(x0, y0, seed);
    auto g10 = gradient(x1, y0, seed);
    auto g01 = gradient(x0, y1, seed);
    auto g11 = gradient(x1, y1, seed);

    auto dot00 = g00.first * dx + g00.second * dy;
    auto dot10 = g10.first * (dx-1) + g10.second * dy;
    auto dot01 = g01.first * dx + g01.second * (dy-1);
    auto dot11 = g11.first * (dx-1) + g11.second * (dy-1);

    auto u = smootherStep(dx);
    auto v = smootherStep(dy);

    return lerp(lerp(dot00, dot10, u), lerp(dot01, dot11, u), v);
}

uint8_t computePerlinAtCoord(int x, int y, int nb_octaves, float persistence,
                      float lacunarity, int grid_size)
{
    auto frequency = 1.f;
    auto amplitude = 1.f;
    float value = 0.f;
    float maxAmp = 0.f;

    for (int i = 0; i < nb_octaves; i++)
    {
        value += amplitude * perlin(
            x / (float)grid_size * frequency,
            y / (float)grid_size * frequency,
            1, 42 * i + 1
        );

        maxAmp  += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    float normalized = (value + 1) / 2;
    normalized = std::clamp(normalized, 0.f, 1.f);
    int gray = (int)(normalized * 255.f);

    return gray;
}

PPM createRandomImage(int sx, int sy, int nb_octaves, float persistence,
                      float lacunarity, int grid_size)
{
    PPM img(sx, sy);

    for (int y = 0; y < sy; y++)
    {
        for (int x = 0; x < sx; x++)
        {
            auto gray = computePerlinAtCoord(x, y, nb_octaves, persistence, lacunarity, grid_size);
            img.pixels[y * sx + x] = Color(gray, gray, gray);
        }
    }

    return img;
}

PPM createWoodTexture(int sx, int sy, int nb_octaves, float persistence,
                      float lacunarity, int grid_size)
{
    PPM img(sx, sy);

    Color darkWood(102, 56, 15);
    Color lightWood(158, 81, 13);

    // Center of the radius
    float cx = sx * -0.5f; // left
    float cy = sy * -1.0f; // above

    for (int y = 0; y < sy; y++)
    {
        for (int x = 0; x < sx; x++)
        {
            float dx = (x - cx) * 0.4f; // rings stretch wide on X
            float dy = (y - cy) * 1.0f; // rings dont stretch on Y
            float dist = std::sqrt(dx * dx + dy * dy);

            float noise = computePerlinAtCoord(x, y, nb_octaves, persistence, lacunarity, grid_size);
            float distortion = (noise / 255.0f - 0.5f) * 120.0f;

            float rings = std::sin((dist + distortion) * 0.12f);

            float t = (rings + 1.0f) / 2.0f;

            img.pixels[y * sx + x] = Color(
                lerp(darkWood.colors[RED], lightWood.colors[RED], t),
                lerp(darkWood.colors[GREEN], lightWood.colors[GREEN], t),
                lerp(darkWood.colors[BLUE], lightWood.colors[BLUE], t)
            );
        }
    }
    return img;
}

PPM createCloudTexture(int sx, int sy, int nb_octaves, float persistence,
                      float lacunarity, int grid_size)
{
    PPM img(sx, sy);
    auto minS = 100;
    auto maxS = 200;
    for (int y = 0; y < sy; y++)
    {
        for (int x = 0; x < sx; x++)
        {
            int gray = computePerlinAtCoord(x, y, nb_octaves, persistence, lacunarity, grid_size);
            if (gray < minS)
                img.pixels[y * sx + x] = Color(61, 174, 235); // blue background
            else if (gray > maxS)
                img.pixels[y * sx + x] = Color(255, 255, 255); // white smoke
            else
            {
                // Linear interpolation from blue -> white as gray goes minS -> maxS
                float interpolation = float(gray - minS) / float(maxS - minS);
                img.pixels[y * sx + x] = Color(
                    lerp(61, 255, interpolation),
                    lerp(174, 255, interpolation),
                    lerp(235, 255, interpolation)
                );
            }
        }
    }
    return img;
}