#include "perlin3D.hh"

#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <vector>

// Compute Random gradient at grid corners
std::vector<float> gradient3D(int ix, int iy, int iz, int seed)
{
    uint32_t h = (uint32_t)ix * 1619u + (uint32_t)iy * 31337u 
               + (uint32_t)iz * 3571u + (uint32_t)seed * 6971u;
    h ^= (h >> 13);
    h *= 1234577u;
    h ^= (h >> 15);
    
    float theta = (h & 0xFFFFu) / 65536.f * M_PI;        // [0, π]
    float phi   = ((h >> 16) & 0xFFFFu) / 65536.f * 2.f * M_PI; // [0, 2π]
    return {
        std::sin(theta) * std::cos(phi),
        std::sin(theta) * std::sin(phi),
        std::cos(theta)
    };
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

float perlin(float x, float y, float z, int grid_size, int seed)
{
    int x0 = (int)std::floor(x / grid_size);
    int y0 = (int)std::floor(y / grid_size);
    int z0 = (int)std::floor(z / grid_size);

    int x1 = x0 + 1;
    int y1 = y0 + 1;
    int z1 = z0 + 1;

    float dx = (x - x0 * grid_size) / grid_size;
    float dy = (y - y0 * grid_size) / grid_size;
    float dz = (z - z0 * grid_size) / grid_size;

    auto g000 = gradient3D(x0, y0, z0, seed);
    auto g100 = gradient3D(x1, y0, z0, seed);
    auto g010 = gradient3D(x0, y1, z0, seed);
    auto g110 = gradient3D(x1, y1, z0, seed);
    auto g001 = gradient3D(x0, y0, z1, seed);
    auto g101 = gradient3D(x1, y0, z1, seed);
    auto g011 = gradient3D(x0, y1, z1, seed);
    auto g111 = gradient3D(x1, y1, z1, seed);

    auto dot000 = g000[0] * dx + g000[1] * dy + g000[2] * dz;
    auto dot100 = g100[0] * (dx-1) + g100[1] * dy + g100[2] * dz;
    auto dot010 = g010[0] * dx + g010[1] * (dy-1) + g010[2] * dz;
    auto dot110 = g110[0] * (dx-1) + g110[1] * (dy-1) + g110[2] * dz;
    auto dot001 = g001[0] * dx + g001[1] * dy + g001[2] * (dz-1);
    auto dot101 = g101[0] * (dx-1) + g101[1] * dy + g101[2] * (dz-1);
    auto dot011 = g011[0] * dx + g011[1] * (dy-1) + g011[2] * (dz-1);
    auto dot111 = g111[0] * (dx-1) + g111[1] * (dy-1) + g111[2] * (dz-1);

    auto u = smootherStep(dx);
    auto v = smootherStep(dy);
    auto w = smootherStep(dz);

    auto a = lerp(lerp(dot000, dot100, u), lerp(dot010, dot110, u), v);
    auto b = lerp(lerp(dot001, dot101, u), lerp(dot011, dot111, u), v);
    return lerp(a, b, w);
}

uint8_t computePerlinAtCoord3D(int x, int y, int z, int nb_octaves, float persistence,
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
            z / (float)grid_size * frequency,
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
