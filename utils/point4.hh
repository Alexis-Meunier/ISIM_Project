#pragma once

#include <iostream>

class Point4
{
    public:
        Point4();
        Point4(const float& x_, const float& y_, const float& z_, const float& i_);
        Point4(const Point4& v);
        Point4(const float& x_, const float& y_, const float& z_);

        void rotateX(const float& angle);
        void rotateY(const float& angle);
        void rotateZ(const float& angle);

        float& operator[](size_t idx);
        float operator[](size_t idx) const;

        float x;
        float y;
        float z;
        float i;
};

inline Point4 operator+(const Point4& p1, const Point4& p2)
{
    return Point4(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z, 1);
}

inline Point4 operator-(const Point4& p1, const Point4& p2)
{
    return Point4(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, 1);
}

inline Point4 operator*(const Point4& p, const double& scalar)
{
    return Point4(p.x * scalar, p.y * scalar, p.z * scalar, 1);
}

inline float operator*(const Point4& p1, const Point4& p2)
{
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}

inline std::ostream& operator<<(std::ostream& os, const Point4& p)
{
    os << "(" << p.x << ", " << p.y << ", " << p.z << ", " << p.i << ")" << std::endl;
    return os;
}

inline float& Point4::operator[](size_t idx) {
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
            throw std::out_of_range("Point4 indexing gone wrong");
    }
}

inline float Point4::operator[](size_t idx) const {
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
            throw std::out_of_range("Point4 indexing gone wrong");
    }
}
