#pragma once

#include "light.hh"

class CircleLight : public Light
{
public:
    CircleLight() = default;

    CircleLight(const Point4& p, const float& power, const float& rad,
                const Vector4& dir = Vector4(0, -1, 0));
    ~CircleLight() override;

    std::vector<Point4> getSamples(float alpha = 0.5f) const;

    float radius;
    Vector4 direction;  // normale au plan du disque
};
