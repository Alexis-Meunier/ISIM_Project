#include "mesh.hh"

#include <fstream>
#include <sstream>
#include <cmath>

Mesh Mesh::rectangle(
    const Point4& top_left,  const Point4& top_right,
    const Point4& bot_right, const Point4& bot_left,
    const std::shared_ptr<TextureMaterial>& texture)
{
    Mesh m;

    Triangle *t1 = new Triangle(top_left, top_right, bot_left, texture);
    t1->uv1 = {0.f, 1.f};
    t1->uv2 = {1.f, 1.f};
    t1->uv3 = {0.f, 0.f};

    Triangle *t2 = new Triangle(top_right, bot_right, bot_left, texture);
    t2->uv1 = {1.f, 1.f};
    t2->uv2 = {1.f, 0.f};
    t2->uv3 = {0.f, 0.f};

    m.triangles.push_back(t1);
    m.triangles.push_back(t2);

    m.build_bvh();
    return m;
}

std::optional<Point4> Mesh::intersect(const Point4& start, const Vector4& dir)
{
    auto [hit, obj] = bvh_intersect(bvh_pool, triangles, start, dir);
    last_hit = dynamic_cast<Triangle*>(obj);
    return hit;
}

Vector4 Mesh::get_normal(const Point4& p)
{
    if (last_hit) return last_hit->get_normal(p);
    return triangles[0]->get_normal(p);
}

TextureInfo* Mesh::get_texture(const Point4& p)
{
    if (last_hit) return last_hit->get_texture(p);
    return triangles[0]->get_texture(p);
}

AABB Mesh::compute_mesh_bounds()
{
    AABB box = triangles[0]->get_bounds();
    for (size_t i = 1; i < triangles.size(); i++)
    {
        auto b = triangles[i]->get_bounds();
        box.min.x = std::min(box.min.x, b.min.x);
        box.min.y = std::min(box.min.y, b.min.y);
        box.min.z = std::min(box.min.z, b.min.z);
        box.max.x = std::max(box.max.x, b.max.x);
        box.max.y = std::max(box.max.y, b.max.y);
        box.max.z = std::max(box.max.z, b.max.z);
    }
    return box;
}

AABB Mesh::get_bounds() const
{
    return cached_bounds;
}

Point4 Mesh::get_centroid() const
{
    float x = 0, y = 0, z = 0;
    for (auto& t : triangles) {
        auto c = t->get_centroid();
        x += c.x; y += c.y; z += c.z;
    }
    float n = (float)triangles.size();
    return Point4(x/n, y/n, z/n);
}

Mesh Mesh::from_obj(const std::string& path,
                        const std::shared_ptr<TextureMaterial>& texture,
                        const Point4& offset, const float& scale, const RotationCoords& rotation)
{
    auto rad = M_PI / 180.0f;
    Mesh m;
    std::vector<Point4>            positions;
    std::vector<std::pair<float, float>> uvs;

    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            auto rotated = Point4(x * scale, y * scale, z * scale);
            if (rotation.x < 1e-3)
                rotated.rotateX(rotation.x * rad);
            if (rotation.y < 1e-3)
                rotated.rotateY(rotation.y * rad);
            if (rotation.z < 1e-3)
                rotated.rotateZ(rotation.z * rad);

            positions.push_back(rotated + offset);
        }
        else if (token == "vt") {
            float u, v;
            ss >> u >> v;
            uvs.push_back({ u, v });
        }
        else if (token == "f") {
            std::vector<int> pi, ui;
            std::string part;
            while (ss >> part) {
                auto slash = part.find('/');
                pi.push_back(std::stoi(part.substr(0, slash)) - 1);
                if (slash != std::string::npos)
                    ui.push_back(std::stoi(part.substr(slash + 1)) - 1);
            }
            for (int i = 1; i + 1 < (int)pi.size(); i++) {
                Triangle *t = new Triangle(positions[pi[0]], positions[pi[i]], positions[pi[i+1]], texture);
                if (!uvs.empty() && (int)ui.size() > i + 1) { // guard both conditions
                    t->uv1 = { uvs[ui[0]].first,   uvs[ui[0]].second };
                    t->uv2 = { uvs[ui[i]].first,   uvs[ui[i]].second };
                    t->uv3 = { uvs[ui[i+1]].first, uvs[ui[i+1]].second };
                }
                m.triangles.push_back(t);
            }
        }
    }

    m.build_bvh();
    return m;
}

void Mesh::build_bvh() {
    bvh_pool.clear();
    bvh_root = build(bvh_pool, triangles, 0, (int)triangles.size());
    cached_bounds = compute_mesh_bounds();
}
