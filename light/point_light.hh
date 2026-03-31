#pragma once

#include <cmath>
#include <vector>

#include "light.hh"

class PointLight : public Light
{
public:
    PointLight() = default;
    PointLight(const Point4& p);
    PointLight(const float& power);
    PointLight(const Point4& p, const float& power);
    PointLight(const Point4& p, const float& power, const Color& color);
    ~PointLight() override;
};
