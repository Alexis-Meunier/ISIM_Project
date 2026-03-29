#pragma once

#include "object.hh"
#include "triangle.hh"
#include <vector>

class Complex : public Object {
    Complex(const std::vector<Triangle*>& triangles);

    std::optional<Point4> intersect(const Point4& start, const Vector4& norm) override;
    Vector4 get_normal(const Point4& p) override;
    TextureInfo get_texture(const Point4& p) override;

    std::vector<Triangle*> faces;
    Triangle *current;
};
