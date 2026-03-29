#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

enum Canal
{
    RED = 0,
    GREEN,
    BLUE,
    GRAY
};

class Color
{
    public:
    Color() : canal(GRAY), value(0) {};
    Color(const Canal& canal_, const uint8_t& value_);
    Color(const Color& c);

    Canal canal;
    uint8_t value;
};

class Pixel
{
    public:
    Pixel();
    Pixel(const uint8_t& r, const uint8_t& g, const uint8_t& b);
    Pixel(const Pixel& p);

    std::vector<uint8_t> colors;    
};

inline Pixel operator+(Pixel& p, const Color& c)
{
    if (c.canal == GRAY)
    {
        return Pixel(p.colors[0] + c.value, p.colors[1] + c.value, p.colors[2] + c.value);
    }
    else
    {
        p.colors[c.canal] += c.value;
        return Pixel(p.colors[0], p.colors[1], p.colors[2]);
    }
}

inline Pixel operator-(Pixel& p, const Color& c)
{
    if (c.canal == GRAY)
    {
        return Pixel(p.colors[0] - c.value, p.colors[1] - c.value, p.colors[2] - c.value);
    }
    else
    {
        p.colors[c.canal] -= c.value;
        return Pixel(p.colors[0], p.colors[1], p.colors[2]);
    }
}

inline std::ostream& operator<<(std::ostream& os, const Pixel& p)
{
    os << "R: " << int(p.colors[0]) << std::endl;
    os << "G: " << int(p.colors[1]) << std::endl;
    os << "B: " << int(p.colors[2]) << std::endl;
    return os;
}
