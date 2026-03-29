#include "point_light.hh"

float clamp(const float& val, const float& lo_, const float& hi_)
{
    if (val < lo_)
        return lo_;
    if (val > hi_)
        return hi_;
    return val;
}

PointLight::PointLight(const Point4& p)
{
    this->power = 1;
    this->position = p;
}


PointLight::PointLight(const float& power)
{
    this->power = clamp(power, 0, 1);
    this->position = Point4(0, 0, 0);
}

PointLight::PointLight(const Point4& p, const float& power)
{
    this->position = p;
    this->power = clamp(power, 0, 1);
}

PointLight::~PointLight()
{

}