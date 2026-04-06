#pragma once

#include "object.hh"

class Sphere : public Object
{
public:
    Sphere();
    Sphere(const double& rad);
    Sphere(const std::shared_ptr<TextureMaterial>& text, const Point4& vec, const double& rad);
    Sphere(const std::shared_ptr<TextureMaterial>& text, const double& rad);
    Sphere(const Point4& vec, const double& rad);

    std::optional<Point4> intersect(const Point4& start, const Vector4& norm) override;
    Vector4 get_normal(const Point4& v) override;
    TextureInfo *get_texture(const Point4& v) override;
    AABB get_bounds() const;
    Point4 get_centroid() const;

    Point4 center;
    double radius;
};
