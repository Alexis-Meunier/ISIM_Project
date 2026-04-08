#pragma once

#include "object.hh"
#include "../utils/point4.hh"

struct UVCoord { float u, v; };

class Triangle : public Object {
public:
    Triangle(const Point4& p1, const Point4& p2, const Point4& p3);
    Triangle(const Point4& p1, const Point4& p2, const Point4& p3, const std::shared_ptr<TextureMaterial>& text);
    Triangle(const std::shared_ptr<TextureMaterial>& text);

    std::optional<Point4> intersect(const Point4& start, const Vector4& norm) override;
    Vector4 get_normal(const Point4& p) override;
    TextureInfo *get_texture(const Point4& p) override;
    AABB get_bounds() const;
    Point4 get_centroid() const;

    AABB compute_my_bounds();

    Point4 p1;
    Point4 p2;
    Point4 p3;
    Vector4 normal;
    Point4 last_hit;
    float last_w0, last_w1, last_w2;
    UVCoord uv1{0,0}, uv2{1,0}, uv3{0.5f,1};
    AABB cached_bounds;
};
