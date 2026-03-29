#include "scene.hh"

Scene::Scene(const std::vector<Object*>& objs, const std::vector<Light*>& ls, const Camera& cam)
    : objects(objs), lights(lights), camera(cam)
{}

void Scene::addObject(Object& obj)
{
    objects.push_back(&obj);
}

void Scene::addLight(Light& light)
{
    lights.push_back(&light);
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
