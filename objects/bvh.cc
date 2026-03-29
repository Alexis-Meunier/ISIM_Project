#include "bvh.hh"

#include "object.hh"

// https://en.wikipedia.org/wiki/Slab_method
bool AABB::intersect(const Point4& origin, const Vector4& dir) const
{
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::max();

    const float* bmin = &min.x;
    const float* bmax = &max.x;

    for (int i = 0; i < 3; i++) {
        if (std::abs(dir[i]) < 1e-8f) {
            if (origin[i] < bmin[i] || origin[i] > bmax[i])
                return false;
        } else {
            float inv = 1.0f / dir[i];
            float t1 = (bmin[i] - origin[i]) * inv;
            float t2 = (bmax[i] - origin[i]) * inv;
            if (t1 > t2)
                std::swap(t1, t2);

            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax)
                return false;
        }
    }

    return true;
}

// Compute the AABB that wraps a list of objects
static AABB compute_bounds(const std::vector<Object*>& objs, int start, int end) {
    AABB box;
    box.min = Point4( 1e30f,  1e30f,  1e30f);
    box.max = Point4(-1e30f, -1e30f, -1e30f);

    for (int i = start; i < end; i++) {
        AABB ob = objs[i]->get_bounds();

        box.min.x = std::min(box.min.x, ob.min.x);
        box.min.y = std::min(box.min.y, ob.min.y);
        box.min.z = std::min(box.min.z, ob.min.z);

        box.max.x = std::max(box.max.x, ob.max.x);
        box.max.y = std::max(box.max.y, ob.max.y);
        box.max.z = std::max(box.max.z, ob.max.z);
    }

    return box;
}

// Centroid of an AABB along axis 0=X, 1=Y, 2=Z
static float centroid(const AABB& b, int axis) {
    return (b.min[axis] + b.max[axis]) * 0.5f;
}

// Recursive build -> returns index of root node
int build(std::vector<BVHNode>& pool, std::vector<Object*>& objs, int start, int end) {
    BVHNode node;
    node.bounds = compute_bounds(objs, start, end);
    int count = end - start;

    // Leaf threshold: max 4 objects (Can change this to go faster - or not - TODO)
    if (count <= 4) {
        node.obj_start = start;
        node.obj_count = count;

        pool.push_back(node);
        return (int)pool.size() - 1;
    }

    // Pick the longest axis to split on
    float dx = node.bounds.max.x - node.bounds.min.x;
    float dy = node.bounds.max.y - node.bounds.min.y;
    float dz = node.bounds.max.z - node.bounds.min.z;
    int axis = (dx >= dy && dx >= dz) ? 0 : (dy >= dz ? 1 : 2);

    // Sort objects by centroid along the chosen axis
    std::sort(objs.begin() + start, objs.begin() + end,
        [axis](Object* a, Object* b) {
            return centroid(a->get_bounds(), axis) < centroid(b->get_bounds(), axis);
        });

    int mid = start + count / 2;

    pool.push_back(node);
    int idx = (int)pool.size() - 1;

    pool[idx].left = build(pool, objs, start, mid);
    pool[idx].right = build(pool, objs, mid, end);

    return idx;
}
