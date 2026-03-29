#include "complex.hh"

Complex::Complex(const std::vector<Triangle*>& triangles)
{
    faces = triangles;
    current = nullptr;
}

std::optional<Point4> Complex::intersect(const Point4& start, const Vector4& norm)
{
    for (auto& triangle : faces)
    {
        auto intersect = triangle->intersect(start, norm);
        if (intersect != std::nullopt)
        {
            current = triangle;
            return intersect;
        }
    }

    return std::nullopt;
}

Vector4 Complex::get_normal(const Point4& p)
{
    if (current == nullptr)
        return Vector4();

    return current->get_normal(p);
}

TextureInfo Complex::get_texture(const Point4& p)
{
    if (current == nullptr)
        return TextureInfo();

    return current->get_texture(p);
}

