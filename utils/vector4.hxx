#pragma once

#include "vector4.hh"

inline Vector4 Vector4::operator+(const Vector4& p)
{
    return Vector4(x + p.x, y + p.y, z + p.z, 1);
}

inline Vector4 Vector4::operator+(const Point4& p) const
{
    return Vector4(x + p.x, y + p.y, z + p.z, 1);
}

inline Vector4 Vector4::operator-()
{
    return Vector4(-x, -y, -z, 1);
}

inline Vector4 Vector4::operator-(const Vector4& p)
{
    return Vector4(x - p.x, y - p.y, z - p.z, 1);
}

inline Vector4 Vector4::operator*(const double& scalar)
{
    return Vector4(x * scalar, y * scalar, z * scalar, 1);
}

inline float Vector4::operator*(const Vector4& p)
{
    return x * p.x + y * p.y + z * p.z;
}

inline Vector4& Vector4::operator=(const Vector4& v) {
    x = v.x; y = v.y; z = v.z; i = 1;
    return *this;
}

inline std::ostream& Vector4::operator<<(std::ostream& os)
{
    os << "(" << x << ", " << y << ", " << z << ", " << i << ")" << std::endl;
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
