#include "scene.hh"

Scene::Scene(const std::vector<Object*>& objs, const Camera& cam)
    : objects(objs), camera(cam)
{}

void Scene::addObject(Object& obj)
{
    objects.push_back(&obj);
}

void Scene::addLights(Object& obj, bool display)
{
    lights.push_back(&obj);
    if (display)
        objects.push_back(&obj);
}

void Scene::setCamera(const Camera& cam)
{
    camera = cam;
}

void Scene::build_bvh() {
    bvh_pool.clear();
    bvh_root = build(bvh_pool, objects, 0, (int)objects.size());
}

// TODO
Scene::~Scene()
{

}
