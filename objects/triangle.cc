#include "triangle.hh"

#include <cmath>

#include "../texture/UniformTexture.hh"
#include "../texture/ImageTexture.hh"

inline Vector4 cross_product(const Vector4& v1, const Vector4& v2)
{
    return Vector4(v1.y * v2.z - v1.z * v2.y, -(v1.x * v2.z - v1.z * v2.x), v1.x * v2.y - v1.y * v2.x);
}

Triangle::Triangle(const Point4& p1, const Point4& p2, const Point4& p3)
{
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    texture = std::make_shared<UniformTexture>();

    auto CB = Vector4(p2.x - p3.x, p2.y - p3.y, p2.z - p3.z);
    auto CA = Vector4(p1.x - p3.x, p1.y - p3.y, p1.z - p3.z);

    this->normal = cross_product(CB, CA);
    cached_bounds = compute_my_bounds();
}

AABB Triangle::compute_my_bounds()
{
    return {
        Point4(std::min({p1.x, p2.x, p3.x}),
               std::min({p1.y, p2.y, p3.y}),
               std::min({p1.z, p2.z, p3.z})),
        Point4(std::max({p1.x, p2.x, p3.x}),
               std::max({p1.y, p2.y, p3.y}),
               std::max({p1.z, p2.z, p3.z}))
    };
}

Triangle::Triangle(const Point4& p1, const Point4& p2, const Point4& p3, const std::shared_ptr<TextureMaterial>& info)
{
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    texture = info;

    auto CB = Vector4(p2.x - p3.x, p2.y - p3.y, p2.z - p3.z);
    auto CA = Vector4(p1.x - p3.x, p1.y - p3.y, p1.z - p3.z);

    this->normal = cross_product(CA, CB);
    cached_bounds = compute_my_bounds();
}

Triangle::Triangle(const std::shared_ptr<TextureMaterial>& info)
{
    this->p1 = Point4(1, 0, 8);
    this->p2 = Point4(0, std::sqrt(3), 8);
    this->p3 = Point4(-1, 0, 8);
    texture = info;

    auto CB = Vector4(p2.x - p3.x, p2.y - p3.y, p2.z - p3.z);
    auto CA = Vector4(p1.x - p3.x, p1.y - p3.y, p1.z - p3.z);

    this->normal = cross_product(CA, CB);
    cached_bounds = compute_my_bounds();
}

std::optional<Point4> Triangle::intersect(const Point4& start, const Vector4& norm)
{
    auto OA = Vector4(p1.x - start.x, p1.y - start.y, p1.z - start.z);
    auto OB = Vector4(p2.x - start.x, p2.y - start.y, p2.z - start.z);
    auto OC = Vector4(p3.x - start.x, p3.y - start.y, p3.z - start.z);

    auto detp = OA.x * OB.y * OC.z + OA.y * OB.z * OC.x + OA.z * OB.x * OC.y;
    auto detn = OC.x * OB.y * OA.z + OC.y * OB.z * OA.x + OB.x * OA.y * OC.z;
    auto det = detp - detn;
    if (std::abs(det) < 1e-5)
    {
        return std::nullopt;
    }

    auto alpha = norm.x * (OB.y * OC.z - OB.z * OC.y) - norm.y * (OB.x * OC.z - OB.z * OC.x) + norm.z * (OB.x * OC.y - OB.y * OC.x);
    auto beta = - norm.x * (OA.y * OC.z - OA.z * OC.y) + norm.y * (OA.x * OC.z - OA.z * OC.x) - norm.z * (OA.x * OC.y - OA.y * OC.x);
    auto gamma = norm.x * (OA.y * OB.z- OB.y * OA.z) - norm.y * (OA.x * OB.z - OA.z * OB.x) + norm.z * (OA.x * OB.y - OA.y * OB.x);

    // 1/det(M) isn't useful because it doesn't change the sign
    if ((alpha < 0 && beta < 0 && gamma < 0) || (alpha > 0 && beta > 0 && gamma > 0))
    {
        auto vec = OA * alpha + OB * beta + OC * gamma;
        auto div = alpha + beta + gamma;
        last_w0 = alpha / div;
        last_w1 = beta  / div;
        last_w2 = gamma / div;

        Point4 hit(start.x + vec.x / div, start.y + vec.y / div, start.z + vec.z / div);

        float tx = hit.x - start.x;
        float ty = hit.y - start.y;
        float tz = hit.z - start.z;
        float t = tx * norm.x + ty * norm.y + tz * norm.z;

        if (t < 1e-4f)
            return std::nullopt;

        return Point4(hit);
    }

    return std::nullopt;
}

Vector4 Triangle::get_normal(const Point4& p)
{
    return this->normal;
}

TextureInfo *Triangle::get_texture(const Point4& p)
{
    auto im = dynamic_cast<ImageTexture*>(texture.get());
    if (!im)
        return texture->get_elements(p);

    float u = last_w0 * uv1.u + last_w1 * uv2.u + last_w2 * uv3.u;
    float v = last_w0 * uv1.v + last_w1 * uv2.v + last_w2 * uv3.v;
    return im->get_elements_uv(u, v);
}

AABB Triangle::get_bounds() const {
    return cached_bounds;
}

// Return random centroid in triangle
Point4 Triangle::get_centroid() const
{
    // float u = static_cast<float>(rand()) / RAND_MAX;
    // float v = static_cast<float>(rand()) / RAND_MAX;
    // if (u + v > 1.f)
    // {
    //     u = 1.f - u; v = 1.f - v;
    // }

    // return Point4(
    //     (1-u-v)*p1.x + u*p2.x + v*p3.x,
    //     (1-u-v)*p1.y + u*p2.y + v*p3.y,
    //     (1-u-v)*p1.z + u*p2.z + v*p3.z
    // );
    return Point4(
        (p1.x + p2.x + p3.x) / 3.f,
        (p1.y + p2.y + p3.y) / 3.f,
        (p1.z + p2.z + p3.z) / 3.f
    );
}
