#pragma once

#include <iostream>

#include "point4.hh"

class Vector4
{
    public:
        Vector4();
        Vector4(const float& x_, const float& y_, const float& z_, const float& i_);
        Vector4(const Vector4& v);
        Vector4(const Point4& p);
        Vector4(const float& x_, const float& y_, const float& z_);

        float& operator[](size_t idx);
        float operator[](size_t idx) const;

        void normalize();

        float x;
        float y;
        float z;
        float i;
};

inline Vector4 operator+(const Vector4& p1, const Vector4& p2)
{
    return Vector4(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z, 1);
}

inline Vector4 operator-(const Vector4& p1, const Vector4& p2)
{
    return Vector4(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, 1);
}

inline Vector4 operator*(const Vector4& p, const double& scalar)
{
    return Vector4(p.x * scalar, p.y * scalar, p.z * scalar,1);
}

inline float operator*(const Vector4& p1, const Vector4& p2)
{
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}

inline std::ostream& operator<<(std::ostream& os, const Vector4& p)
{
    os << "(" << p.x << ", " << p.y << ", " << p.z << ", " << p.i << ")" << std::endl;
    return os;
}

inline float& Vector4::operator[](size_t idx) {
    switch(idx)
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
            throw std::out_of_range("Vector4 indexing gone wrong");
    }
}

inline float Vector4::operator[](size_t idx) const {
    switch(idx)
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
            throw std::out_of_range("Vector4 indexing gone wrong");
    }
}
