#include "texture/ProceduralTexture.hh"

#include <cmath>

ProceduralTexture::ProceduralTexture(ProceduralType type, float kd, float ks,
                float lightPower, bool isLight)
{
    this->type = type;
    info = new TextureInfo();
    info->kd = kd;
    info->ks = ks;
    info->lightPower = lightPower;
    info->color = new Color();
    this->isLight = isLight;
}

Color *woodTextureAtXY(int x, int y, int z)
{
    Color darkWood(102, 56, 15);
    Color lightWood(158, 81, 13);

    // Center of the radius
    float cx = -50; // left
    float cy = -100; // above

    float dx = (x - cx) * 0.4f; // rings stretch wide on X
    float dy = (y - cy) * 1.0f; // rings dont stretch on Y
    float dist = std::sqrt(dx * dx + dy * dy);

    float noise = computePerlinAtCoord3D(x, y, z, 6, 0.7, 2, 120);
    float distortion = (noise / 255.0f - 0.5f) * 120.0f;

    float rings = std::sin((dist + distortion) * 0.12f);

    float t = (rings + 1.0f) / 2.0f;

    return new Color(
        lerp(darkWood.colors[RED], lightWood.colors[RED], t),
        lerp(darkWood.colors[GREEN], lightWood.colors[GREEN], t),
        lerp(darkWood.colors[BLUE], lightWood.colors[BLUE], t)
    );
}

Color *cloudTextureAtXY(int x, int y, int z)
{
    auto minS = 60;
    auto maxS = 200;

    int gray = computePerlinAtCoord3D(x, y, z, 4, 0.6, 2, 200);

    if (gray < minS)
        return new Color(61, 174, 235); // blue background
    else if (gray > maxS)
        return new Color(255, 255, 255); // white smoke
    else
    {
        // Linear interpolation from blue -> white as gray goes minS -> maxS
        float interpolation = float(gray - minS) / float(maxS - minS);
        return new Color(
            lerp(61, 255, interpolation),
            lerp(174, 255, interpolation),
            lerp(235, 255, interpolation)
        );
    }
}

TextureInfo *ProceduralTexture::get_elements(Point4 position)
{
    if (type == WOOD)
    {
        info->color = woodTextureAtXY(3 * position.x, 3 * position.y, 3 * position.z);
        return info;
    }
    else if (type == CLOUD)
    {
        info->color = cloudTextureAtXY(5 * position.x, 5 * position.y, 5 * position.z);
        return info;
    }

    return info;
}