#pragma once

#include "point4.hh"

inline Point4 Point4::operator+(const Point4& p) const
{
    return Point4(x + p.x, y + p.y, z + p.z, 1);
}

inline Point4 Point4::operator-(const Point4& p) const
{
    return Point4(x - p.x, y - p.y, z - p.z, 1);
}

inline Point4 Point4::operator*(const double& scalar) const
{
    return Point4(x * scalar, y * scalar, z * scalar, 1);
}

inline float Point4::operator*(const Point4& p) const
{
    return x * p.x + y * p.y + z * p.z;
}

inline Point4& Point4::operator=(const Point4& v) {
    x = v.x; y = v.y; z = v.z; i = 1;
    return *this;
}

inline std::ostream& Point4::operator<<(std::ostream& os)
{
    os << "(" << x << ", " << y << ", " << z << ", " << i << ")" << std::endl;
    return os;
}

inline float& Point4::operator[](size_t idx)
{
    switch (idx)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return i;
    default:
        throw std::out_of_range("Point4 indexing gone wrong");
    }
}

inline float Point4::operator[](size_t idx) const
{
    switch (idx)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return i;
    default:
        throw std::out_of_range("Point4 indexing gone wrong");
    }
}
