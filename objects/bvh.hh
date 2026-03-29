#pragma once

#include "../utils/vector4.hh"
#include "../utils/point4.hh"

#include <vector>
#include <algorithm>
#include <limits>

// https://en.wikipedia.org/wiki/Minimum_bounding_box#Axis-aligned_minimum_bounding_box
struct AABB {
    Point4 min, max;

    // https://en.wikipedia.org/wiki/Slab_method
    bool intersect(const Point4& origin, const Vector4& dir) const;
};

// https://en.wikipedia.org/wiki/Bounding_volume_hierarchy
struct BVHNode {
    AABB bounds;
    int left = -1;
    int right = -1;

    int obj_start = -1;
    int obj_count = 0;

    bool is_leaf() const { return obj_count > 0; };
};

class Object;

int build(std::vector<BVHNode>& pool, std::vector<Object*>& objs, int start, int end);
