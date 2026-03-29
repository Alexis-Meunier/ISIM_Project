#include "point4.hh"

#include <cmath>

Point4::Point4()
    : x(0), y(0), z(0), i(0)
{}

Point4::Point4(const float& x_, const float& y_, const float& z_, const float& i_)
{
    this->x = x_;
    this->y = y_;
    this->z = z_;
    this->i = 1;
}

Point4::Point4(const float& x_, const float& y_, const float& z_)
{
    this->x = x_;
    this->y = y_;
    this->z = z_;
    this->i = 1;
}

Point4::Point4(const Point4& v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->i = 1;
}

void Point4::rotateX(const float& angle)
{
    this->y = std::cos(angle) * y - std::sin(angle) * z;
    this->z = std::sin(angle) * y + std::cos(angle) * z;
}

void Point4::rotateY(const float& angle)
{
    this->x = std::cos(angle) * x - std::sin(angle) * z;
    this->z = std::sin(angle) * x + std::cos(angle) * z;
}

void Point4::rotateZ(const float& angle)
{
    this->x = std::cos(angle) * x - std::sin(angle) * y;
    this->y = std::sin(angle) * x + std::cos(angle) * y;
}
