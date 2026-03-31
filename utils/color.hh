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

class Value
{
    public:
    Value() : canal(GRAY), value(0) {};
    Value(const Canal& canal_, const uint8_t& value_);
    Value(const Value& c);

    Canal canal;
    uint8_t value;
};

class Color
{
    public:
    Color();
    Color(const uint8_t& r, const uint8_t& g, const uint8_t& b);
    Color(const Color& p);

    std::vector<uint8_t> colors;    
};

inline Color operator+(Color& p, const Value& c)
{
    if (c.canal == GRAY)
    {
        return Color(p.colors[0] + c.value, p.colors[1] + c.value, p.colors[2] + c.value);
    }
    else
    {
        p.colors[c.canal] += c.value;
        return Color(p.colors[0], p.colors[1], p.colors[2]);
    }
}

inline Color operator-(Color& p, const Value& c)
{
    if (c.canal == GRAY)
    {
        return Color(p.colors[0] - c.value, p.colors[1] - c.value, p.colors[2] - c.value);
    }
    else
    {
        p.colors[c.canal] -= c.value;
        return Color(p.colors[0], p.colors[1], p.colors[2]);
    }
}

inline std::ostream& operator<<(std::ostream& os, const Color& p)
{
    os << "R: " << int(p.colors[0]) << std::endl;
    os << "G: " << int(p.colors[1]) << std::endl;
    os << "B: " << int(p.colors[2]) << std::endl;
    return os;
}
