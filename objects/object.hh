#pragma once

#include "bvh.hh"

#include "../texture/texture.hh"
#include "../utils/vector4.hh"
#include "../utils/point4.hh"

#include <optional>
#include <memory>

class Object
{
public:
    Object() = default;

    virtual std::optional<Point4> intersect(const Point4& start, const Vector4& norm) = 0;
    virtual Vector4 get_normal(const Point4& p) = 0;
    virtual TextureInfo get_texture(const Point4& p) = 0;
    virtual AABB get_bounds() const = 0;

    std::shared_ptr<TextureMaterial> texture = nullptr;
};
