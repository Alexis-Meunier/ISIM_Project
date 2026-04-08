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
    float oldY = this->y, oldZ = this->z;
    this->y = std::cos(angle) * oldY - std::sin(angle) * oldZ;
    this->z = std::sin(angle) * oldY + std::cos(angle) * oldZ;
}

void Point4::rotateY(const float& angle)
{
    float oldX = this->x, oldZ = this->z;
    this->x = std::cos(angle) * oldX - std::sin(angle) * oldZ;
    this->z = std::sin(angle) * oldX + std::cos(angle) * oldZ;
}

void Point4::rotateZ(const float& angle)
{
    float oldX = this->x, oldY = this->y;
    this->x = std::cos(angle) * oldX - std::sin(angle) * oldY;
    this->y = std::sin(angle) * oldX + std::cos(angle) * oldY;
}

float distance(const Point4& p1, const Point4& p2)
{
    // Norm computation
    return std::sqrt(
        std::pow(p1.x - p2.x, 2) +
        std::pow(p1.y - p2.y, 2) +
        std::pow(p1.z - p2.z, 2)
    );
}
