#pragma once

#include <vector>

#include "../camera/camera.hh"
#include "../light/light.hh"
#include "../objects/object.hh"

class Scene
{
public:
    Scene() = default;
    Scene(const std::vector<Object*>& objs, const Camera& cam);
    ~Scene();

    void addObject(Object& obj);
    void addLights(Object& obj);
    void setCamera(const Camera& cam);

    void build_bvh();

    std::vector<Object*> objects;
    std::vector<Object*> lights;
    Camera camera;

    // The bounding boxes in the scene
    std::vector<BVHNode> bvh_pool;
    int bvh_root = -1;
};
