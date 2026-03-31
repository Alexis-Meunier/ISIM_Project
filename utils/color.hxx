#pragma once

#include "color.hh"


inline Color Color::operator+(const Value& c)
{
    if (c.canal == GRAY)
    {
        return Color(colors[0] + c.value, colors[1] + c.value, colors[2] + c.value);
    }
    else
    {
        colors[c.canal] += c.value;
        return Color(colors[0], colors[1], colors[2]);
    }
}

inline Color Color::operator-(const Value& c)
{
    if (c.canal == GRAY)
    {
        return Color(colors[0] - c.value, colors[1] - c.value, colors[2] - c.value);
    }
    else
    {
        colors[c.canal] -= c.value;
        return Color(colors[0], colors[1], colors[2]);
    }
}

inline Color Color::operator*()
{
    return *this;
}

inline uint8_t Color::operator[](const int& idx) const
{
    if (idx < 0 || idx > 3)
    {
        throw std::runtime_error("Wrong error for color indexing");
    }

    return colors[idx];
}


inline uint8_t& Color::operator[](const int& idx)
{
    if (idx < 0 || idx > 3)
    {
        throw std::runtime_error("Wrong error for color indexing");
    }

    return colors[idx];
}

inline std::ostream& Color::operator<<(std::ostream& os)
{
    os << "R: " << int(colors[0]) << std::endl;
    os << "G: " << int(colors[1]) << std::endl;
    os << "B: " << int(colors[2]) << std::endl;
    return os;
}