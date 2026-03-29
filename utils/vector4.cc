#include "vector4.hh"

#include <cmath>

Vector4::Vector4()
    : x(0), y(0), z(0), i(0)
{}

Vector4::Vector4(const float& x_, const float& y_, const float& z_, const float& i_)
{
    this->x = x_;
    this->y = y_;
    this->z = z_;
    this->i = i_;
}

Vector4::Vector4(const float& x_, const float& y_, const float& z_)
{
    this->x = x_;
    this->y = y_;
    this->z = z_;
    this->i = 1;
}

Vector4::Vector4(const Vector4& v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->i = 1;
}

Vector4::Vector4(const Point4& p)
{
    this->x = p.x;
    this->y = p.y;
    this->z = p.z;
    this->i = 1;
}

void Vector4::normalize()
{
    float norm = std::sqrt(x * x + y * y + z * z);
    if (norm < 1e-8f)
    {
        this->x = this->y = this->z = 0;
        this->i = 1;
    }
    else
    {
        this->x = x / norm;
        this->y = y / norm;
        this->z = z / norm;
        this->i = 1;
    }
}

