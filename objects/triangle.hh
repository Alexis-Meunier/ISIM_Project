#pragma once

#include "object.hh"
#include "../utils/point4.hh"

class Triangle : public Object {
public:
    Triangle(const Point4& p1, const Point4& p2, const Point4& p3);
    Triangle(const Point4& p1, const Point4& p2, const Point4& p3, const UniformTexture& text);
    Triangle(const UniformTexture& text);

    std::optional<Point4> intersect(const Point4& start, const Vector4& norm) override;
    Vector4 get_normal(const Point4& p) override;
    TextureInfo get_texture(const Point4& p) override;
    AABB get_bounds() const;

    Point4 p1;
    Point4 p2;
    Point4 p3;
    Vector4 normal;
};
