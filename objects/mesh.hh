#pragma once

#include "object.hh"
#include "triangle.hh"

#include <vector>

struct RotationCoords
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
};

class Mesh : public Object
{
public:
    static Mesh rectangle(
        const Point4& bot_left,
        const Point4& bot_right,
        const Point4& top_right,
        const Point4& top_left,
        const std::shared_ptr<TextureMaterial>& texture);

    static Mesh from_obj(const std::string& path,
                            const std::shared_ptr<TextureMaterial>& texture,
                        const Point4& offset = Point4(), const float& scale = 1.f, const RotationCoords& rotation = RotationCoords());

    std::optional<Point4> intersect(const Point4& start, const Vector4& dir) override;
    Vector4 get_normal(const Point4& p) override;
    TextureInfo* get_texture(const Point4& p) override;
    AABB get_bounds() const override;
    Point4 get_centroid() const override;
    AABB compute_mesh_bounds();


private:
    void build_bvh();

    std::vector<Object*> triangles;
    Triangle* last_hit = nullptr; // which triangle was hit last
    
    std::vector<BVHNode> bvh_pool;
    int bvh_root = -1;

    AABB cached_bounds;
};
