#include "sphere.hh"

#include <cmath>

#include "../texture/UniformTexture.hh"

Sphere::Sphere()
{
    center = Point4();
    texture = std::make_shared<UniformTexture>();
    radius = 1;
}

Sphere::Sphere(const double& rad)
{
    if (rad <= 0)
    {
        throw new std::runtime_error("Radius is negative for the sphere");
    }

    center = Point4();
    texture = std::make_shared<UniformTexture>();
    radius = rad;
}

Sphere::Sphere(const std::shared_ptr<TextureMaterial>& text, const Point4& vec, const double& rad)
{
    if (rad <= 0)
    {
        throw new std::runtime_error("Radius is negative for the sphere");
    }

    center = vec;
    texture = text;
    radius = rad;
}

Sphere::Sphere(const Point4& vec, const double& rad)
{
    if (rad <= 0)
    {
        throw new std::runtime_error("Radius is negative for the sphere");
    }

    center = vec;
    texture = std::make_shared<UniformTexture>();
    radius = rad;
}


Sphere::Sphere(const std::shared_ptr<TextureMaterial>& text, const double& rad)
{
    if (rad <= 0)
    {
        throw new std::runtime_error("Radius is negative for the sphere");
    }

    center = Point4();
    texture = text;
    radius = rad;
}

// TODO: This was made by hand, there might be faster algorithms ?
std::optional<Point4> Sphere::intersect(const Point4& start, const Vector4& norm)
{
    float dx = start.x - center.x;
    float dy = start.y - center.y;
    float dz = start.z - center.z;

    float a = std::pow(norm.x, 2) + std::pow(norm.y, 2) + std::pow(norm.z, 2);
    float b = 2 * (norm.x * dx + norm.y * dy + norm.z * dz);
    float c = std::pow(dx, 2) + std::pow(dy, 2) + std::pow(dz, 2) - std::pow(radius, 2);

    float delta = std::pow(b, 2) - 4 * a * c;
    if (delta < 0)
    {
        return std::nullopt;
    }

    float coef;
    if (delta == 0)
    {
        coef = -(b / (2 * a));
    }
    else
    {
        float a1 = (-b - std::sqrt(delta)) / (2 * a);
        float a2 = (-b + std::sqrt(delta)) / (2 * a);

        if (a1 > 0 && a2 > 0)
            coef = std::min(a1, a2);
        else if (a1 > 0)
            coef = a1;
        else if (a2 > 0)
            coef = a2;
        else {
            // std::cout << "Both null" << std::endl;
            return std::nullopt;
        }
    }
    return Point4(start.x + norm.x * coef, start.y + norm.y * coef, start.z + norm.z * coef);
}

Vector4 Sphere::get_normal(const Point4& intersection)
{
    return Vector4(intersection.x - center.x, intersection.y - center.y, intersection.z - center.z);
}

TextureInfo Sphere::get_texture(const Point4& v)
{
    return texture->get_elements(v);
}

AABB Sphere::get_bounds() const {
    return { Point4(center.x - radius, center.y - radius, center.z - radius),
             Point4(center.x + radius, center.y + radius, center.z + radius) };
}
