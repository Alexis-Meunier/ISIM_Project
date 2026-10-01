#include "objects/sphere.hh"

#include <cmath>

#include "texture/UniformTexture.hh"
#include "texture/ImageTexture.hh"
#include "texture/ProceduralTexture.hh"

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
            return std::nullopt;
        }
    }
    return Point4(start.x + norm.x * coef, start.y + norm.y * coef, start.z + norm.z * coef);
}

Vector4 Sphere::get_normal(const Point4& intersection)
{
    return Vector4(intersection.x - center.x, intersection.y - center.y, intersection.z - center.z);
}

TextureInfo *Sphere::get_texture(const Point4& p)
{
    float nx = (p.x - center.x) / radius;
    float nz = (p.z - center.z) / radius;

    float phi = std::atan2(nz, nx);;
    float theta = std::acos(nz);

    float u = (phi + M_PI) / (2.0f * M_PI);
    float v = theta / M_PI;

    auto im = dynamic_cast<ImageTexture*>(texture.get());
    if (!im)
    {
        auto proc = dynamic_cast<ProceduralTexture*>(texture.get());
        if (!proc)
            return texture->get_elements(p);

        Vector4 vec = Vector4(p - center);
        vec.normalize();

        float u = 0.5 + atan2(vec.z, vec.x) / (2 * M_PI);
        float v = 0.5 - asin(vec.y) / M_PI;

        return proc->get_elements(Point4(u * 1000, 0, v * 1000));
    }

    auto val = im->get_elements_uv(u, v);
    return val;
}

AABB Sphere::get_bounds() const {
    return { Point4(center.x - radius, center.y - radius, center.z - radius),
             Point4(center.x + radius, center.y + radius, center.z + radius) };
}

Point4 Sphere::get_centroid() const
{
    float u = (static_cast<float>(rand()) / RAND_MAX) * 2.f * M_PI;
    float v = (static_cast<float>(rand()) / RAND_MAX) * M_PI;
    Point4 sample(
        center.x + radius * std::sin(v) * std::cos(u),
        center.y + radius * std::sin(v) * std::sin(u),
        center.z + radius * std::cos(v)
    );

    return sample;
}
