#pragma once

#include <cmath>
#include <vector>

#include "utils/vector4.hh"
#include "utils/point4.hh"
#include "utils/color.hh"

class Light
{
public:
    Light() = default;
    virtual ~Light() = default;

    Point4 position;
    float power;
    Color color;
};
