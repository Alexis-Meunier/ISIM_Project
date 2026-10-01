#include "light/circle_light.hh"

#include <random>

inline Vector4 cross_product(const Vector4& v1, const Vector4& v2)
{
    return Vector4(v1.y * v2.z - v1.z * v2.y, -(v1.x * v2.z - v1.z * v2.x), v1.x * v2.y - v1.y * v2.x);
}

std::pair<Vector4, Vector4> buildBasis(const Vector4& normal)
{
    Vector4 n = normal;
    n.normalize();

    Vector4 arbitrary = (std::abs(n.x) < 0.9f) ? Vector4(1, 0, 0) : Vector4(0, 1, 0);

    Vector4 u = cross_product(n, arbitrary);
    u.normalize();

    Vector4 v = cross_product(n, u);
    v.normalize();

    return {u, v};
}

std::vector<Point4> sampleDisk(const Point4& center,
                                const Vector4& normal,
                                float radius,
                                float alpha)
{
    auto [u, v] = buildBasis(normal);
    std::vector<Point4> samples;

    int N = static_cast<int>(std::ceil(M_PI / (alpha * alpha)));

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (int i = 0; i < N; ++i)
    {
        float u_rand = dist(rng);
        float v_rand = dist(rng);

        float r = radius * std::sqrt(u_rand);
        float theta = 2.f * M_PI * v_rand;

        samples.push_back(Point4(
            center.x + r * (std::cos(theta) * u.x + std::sin(theta) * v.x),
            center.y + r * (std::cos(theta) * u.y + std::sin(theta) * v.y),
            center.z + r * (std::cos(theta) * u.z + std::sin(theta) * v.z)
        ));
    }

    return samples;
}

CircleLight::CircleLight(const Point4& p, const float& power, const float& rad, const Vector4& dir)
{
    position = p;
    this->power = power;
    color = Color();
    radius = rad;
    direction = dir;
}

std::vector<Point4> CircleLight::getSamples(float alpha) const
{
    return sampleDisk(position, direction, radius, alpha);
}

CircleLight::~CircleLight()
{

}
