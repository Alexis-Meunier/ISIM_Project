#include "moteur.hh"

#include <algorithm>
#include <cmath>
#include <random>
#include <memory>
#include <ctime>

#include "utils/scene.hh"
#include "image/image.hh"
#include "objects/sphere.hh"
#include "objects/triangle.hh"
#include "light/point_light.hh"
#include "light/circle_light.hh"
#include "texture/UniformTexture.hh"

template <typename T, typename U>
using pair = std::pair<T, U>;

template <typename T>
using optional = std::optional<T>;

template <typename T>
using vector = std::vector<T>;

int val = -1;

void printProgressBar(int progress, int total, int barWidth = 50) {
    static int lastPercent = -1;
    int percent = (int)((float)progress / total * 100);
    if (percent == lastPercent) return;
    lastPercent = percent;

    int filled = percent * barWidth / 100;

    std::string bar(filled, '#');
    bar = "\033[32m" + bar;
    bar += "\033[31m";
    bar += std::string(barWidth - filled, '-');
    bar += "\033[0m";

    printf("\r[%s] %d%%", bar.c_str(), percent);
    fflush(stdout);
}

struct BRDF
{
    Vector4 V; // View vector
    Vector4 N; // Normal vector
    Vector4 L; // Light vector
    Vector4 H; // Half between V and L
    Vector4 R; // Perfect Reflected ray
};

struct LightComputation
{
    Color *color = nullptr;
    float kd = 0; // Diffuse reflection
    float ks = 0; // Specular reflection
    float li = 0; // Light power
    float ns = 0; // Shininess factor
    float nl = 0; // Dot Product between light and normal vector 
    float li_red = 0; // Light power for the RED Canal
    float li_blue = 0; // Light power for the BLUE Canal
    float li_green = 0; // Light power for the GREEN Canal
    float products = 0; // Dot Product between light and reflected vector
    float contribution_red = 0; // Contribution for the RED canal of other objects
    float contribution_blue = 0; // Contribution for the BLUE canal of other objects
    float contribution_green = 0; // Contribution for the GREEN canal of other objects
};

/**
 * Computes the diffuse light received using the parameters:
 * 
 * light = kd * color * intensity * (N.Li)
 *
 * with:
 *      - kd = the amount of light kept in [0, 1]
 *      - color = the amount of color in the interval [0, 1]
 *      - intensity = the power of the light in [0, 1]
 *      - N = the normal to the point hit by the ray
 *      - Li = the ray sent towards the light
 */
float compute_diffuse_light(LightComputation& variables, const int& canal)
{
    variables.li = variables.li > 1 ? 1 : variables.li;
    variables.nl = variables.nl > 1 ? 1 : variables.nl;

    auto color = *(variables.color);
    auto val = variables.kd * (color[canal] / 255.) * variables.li * variables.nl;
    return val > 1 ? 1 : val;
}

/**
 * Computes the specular light received using the parameters:
 * 
 * light = ks * color * intensity * (N.Li)^ns
 *
 * with:
 *      - kd = the amount of light kept in [0, 1]
 *      - color = the amount of color in the interval [0, 1]
 *      - intensity = the power of the light in [0, 1]
 *      - N = the normal to the point hit by the ray
 *      - Li = the ray sent towards the light
 */
float compute_specular_light(LightComputation& variables, const int& canal)
{
    variables.li = variables.li > 1 ? 1 : variables.li;
    variables.products = variables.products > 1 ? 1 : variables.products;

    auto color = *(variables.color);
    auto val = variables.ks * (color[canal] / 255.) * variables.li * std::pow(variables.products, variables.ns);
    return val > 1 ? 1 : val;
}

Color compute_color(LightComputation& variables)
{
    // auto brdfLambertian = (variables.kd / std::PI) * dot(brdf.N,brdf.L)

    // Compute diffuse color
    auto diff_red = compute_diffuse_light(variables, RED);
    auto diff_green = compute_diffuse_light(variables, GREEN);
    auto diff_blue = compute_diffuse_light(variables, BLUE);

    // Compute specular color
    auto spec_red = compute_specular_light(variables, RED);
    auto spec_green = compute_specular_light(variables, GREEN);
    auto spec_blue = compute_specular_light(variables, BLUE);

    // Compute contribution color
    auto contr_red = variables.li_red * variables.nl;
    auto contr_green = variables.li_green * variables.nl;
    auto contr_blue = variables.li_blue * variables.nl;

    // Sum all 3
    auto red = std::min(1.0f, diff_red + spec_red + contr_red);
    auto green = std::min(1.0f, diff_green + spec_green + contr_green);
    auto blue = std::min(1.0f, diff_blue + spec_blue + contr_blue);

    return Color(255 * red, 255 * green, 255 * blue);
}

std::pair<optional<Point4>, Object*> bvh_intersect(const vector<BVHNode>& pool, const vector<Object*>& objs, const Point4& origin, const Vector4& dir, int node_idx = 0)
{
    const BVHNode& node = pool[node_idx];

    if (!node.bounds.intersect(origin, dir))
        return {std::nullopt, nullptr};

    if (node.is_leaf()) {
        // Linear search within the leaf (at most 4 objects for now)
        float best_dist = std::numeric_limits<float>::max();
        optional<Point4> best_hit = std::nullopt;
        Object *best_obj = nullptr;

        for (int i = node.obj_start; i < node.obj_start + node.obj_count; i++) {
            auto hit = objs[i]->intersect(origin, dir);
            if (!hit)
                continue;

            float d = distance(origin, *hit);
            if (d < best_dist) {
                best_dist = d;
                best_hit = hit;
                best_obj = objs[i];
            }
        }

        return {best_hit, best_obj};
    }

    // Recurse into both children, keep the closer hit
    auto [lhit, lobj] = bvh_intersect(pool, objs, origin, dir, node.left);
    auto [rhit, robj] = bvh_intersect(pool, objs, origin, dir, node.right);

    if (!lhit && !rhit)
        return {std::nullopt, nullptr};
    if (!lhit)
        return {rhit, robj};
    if (!rhit)
        return {lhit, lobj};

    return distance(origin, *lhit) < distance(origin, *rhit)
        ? std::make_pair(lhit, lobj)
        : std::make_pair(rhit, robj);
}

Color cast_ray(const Point4& hit, Object* obj, Scene& scene, int depth)
{
    if (depth == 5)
        return Color();

    // Instantiate all variables we need to compute the color of the pixels
    TextureInfo info = obj->get_texture(hit);
    LightComputation variables;
    variables.color = info.color;
    variables.kd = info.kd;
    variables.ks = info.ks;
    variables.ns = info.ns;

    // Compute normal vector
    auto normal_vect = obj->get_normal(hit);
    normal_vect.normalize();

    // Compute view vector
    auto V = Vector4(scene.camera.center - hit);
    V.normalize();

    // Compute reflected ray
    auto R = normal_vect * 2.0 * dot_product(normal_vect, V) - V;
    R.normalize();

    // Compute light contribution for each light source
    for (Light *light : scene.lights)
    {
        float r = light->color.colors[RED]   / 255.f;
        float g = light->color.colors[GREEN] / 255.f;
        float b = light->color.colors[BLUE]  / 255.f;

        // Tint: how much each channel is boosted relative to a white light
        variables.li += light->power;
        variables.li_red += light->power * r;
        variables.li_blue += light->power * g;
        variables.li_green += light->power * b;
        // std::cout << "light red: c << int(light->color.colors[GREEN]) << std::endl;

        std::vector<Point4> lightSamples;
        if (auto* cl = dynamic_cast<CircleLight*>(light))
        {
            lightSamples = cl->getSamples(0.5f); // Change value to change result

            auto test = lightSamples[0];
            auto test_v = Vector4(test - hit);
            test_v.normalize();

            // If the surface is behind the light direction, we ignore it
            if (dot_product(test_v, cl->direction) > 0)
            {
                continue;
            }
        }
        else
            lightSamples.push_back(light->position); // Point light --> One source


        float visibleCount = 0;
        float nl_local = 0;
        float spec_local = 0;

        // For each sample, check if it's visible from the hit point
        for (auto& sample : lightSamples)
        {
            // Compute light vector
            auto Li = Vector4(sample - hit);
            Li.normalize();

            // Check if ray is intercepted by another object
            bool in_shadow = false;
            auto [shadow_hit, shadow_obj] = bvh_intersect(scene.bvh_pool, scene.objects, hit, Li);
            if (shadow_hit && shadow_obj != obj)
            {
                auto to_light   = distance(hit, sample);
                auto to_blocker = distance(hit, *shadow_hit);
                if (to_blocker > 0.01f && to_blocker < to_light)
                    in_shadow = true;
            }

            // If was intercepted add values
            if (!in_shadow)
            {
                visibleCount += 1.0f;
                nl_local += dot_product(Li, normal_vect);

                auto S = normal_vect * 2.0 * dot_product(normal_vect, Li) - Li;
                S.normalize();

                spec_local += dot_product(S, V);
            }
        }

        // Average values for each sample
        float nSamples = static_cast<float>(lightSamples.size());
        float visibility = visibleCount / nSamples;

        variables.nl += visibility * (nl_local / nSamples);
        variables.products += visibility * (spec_local / nSamples);
    }

    // Recursively call the function for other objects contribution
    auto [refl_hit, refl_obj] = bvh_intersect(scene.bvh_pool, scene.objects, hit, R);
    if (refl_hit && refl_obj != obj)
    {
        auto new_color = cast_ray(*refl_hit, refl_obj, scene, depth + 1);
        variables.contribution_red   += info.ks * new_color.colors[RED] / 255.;
        variables.contribution_green += info.ks * new_color.colors[GREEN] / 255.;
        variables.contribution_blue  += info.ks * new_color.colors[BLUE] / 255.;
    }

    return compute_color(variables, info);
}

std::pair<Vector4, Vector4> get_basis(const Camera& cam)
{
    auto straight = cam.looking_at;
    auto up = cam.up;

    if (straight * up > 0.999)
        up = Vector4(0, 0, 1);

    // S = U.V
    auto horizontal = cross_product(straight, up);
    horizontal.normalize();

    // U' = S.U
    auto real_up = cross_product(horizontal, straight);
    real_up.normalize();

    return {horizontal, real_up};
}

std::pair<float, float> get_camera_plane(const Camera& cam)
{
    float dist_to_zmin = distance(cam.zmin, cam.center);

    // Physics: I drew it on a board to be sure
    float W = std::tan(cam.open_angle_x) * dist_to_zmin;
    float H = std::tan(cam.open_angle_y) * dist_to_zmin;

    return {W, H};
}

PPM computeScene(Scene& scene, const int& image_h, const int& image_w)
{
    PPM img(image_h, image_w);

    Camera cam = scene.camera;

    // Get dimensions of the camera plane
    auto [W, H] = get_camera_plane(cam);

    // Increment depending on resulting image resolution
    auto x_incr = (2 * W) / image_w;
    auto y_incr = (2 * H) / image_h;

    // Get camera plane basis
    auto [horizontal, real_up] = get_basis(cam);

    scene.build_bvh();

    for (int i = 0; i < image_h; i++)
    {
        for (int j = 0; j < image_w; j++)
        {
            // Compute the position of the pixel in the camera plane
            float y = H - (i + 0.5f) * y_incr;
            float x = W - (j + 0.5f) * x_incr;

            // Anti-aliasing --> Send multiple random rays
            int red = 0, green = 0, blue = 0;
            for (int k = 0; k < NB_RAYS; k++)
            {
                float r1 = -0.5f * x_incr + (static_cast<float>(rand()) / RAND_MAX) * x_incr;
                float r2 = -0.5f * y_incr + (static_cast<float>(rand()) / RAND_MAX) * y_incr;

                // Get pixel position and ray sent through it
                Point4 pixel(
                    cam.zmin.x + (x + r1) * horizontal.x + (y + r2) * real_up.x,
                    cam.zmin.y + (x + r1) * horizontal.y + (y + r2) * real_up.y,
                    cam.zmin.z + (x + r1) * horizontal.z + (y + r2) * real_up.z
                );
                Vector4 ray(
                    pixel.x - cam.center.x,
                    pixel.y - cam.center.y,
                    pixel.z - cam.center.z
                );
                ray.normalize();

                // Find first hit object
                auto [intersection, obj] = bvh_intersect(scene.bvh_pool, scene.objects, cam.center, ray);
                if (intersection == std::nullopt)
                    continue;

                // Compute its color and add it to the pixel color
                auto color = cast_ray(*intersection, obj, scene, 0);
                red += color.colors[RED];
                green += color.colors[GREEN];
                blue += color.colors[BLUE];
            }

            // Average values of each ray
            img.pixels[i * image_w + j] = new Color(red / NB_RAYS, green / NB_RAYS, blue / NB_RAYS);
            printProgressBar(i * image_w + j, image_w * image_h);
        }
    }

    printProgressBar(50, 50);
    return img;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./tp1 <filename>" << std::endl;
        return 1;
    }

    time_t load_start = std::time(nullptr);
    TextureInfo flat_random{kd: 0.8, ks: 0.2, ns: 0.8, color: new Color(68, 164, 112)};
    TextureInfo mat_red{kd: 0.5, ks: 0.2, ns: 0.9, color: new Color(255, 0, 0)};

    auto uniform_flat_red = std::make_shared<UniformTexture>(Color(255, 0, 0));
    auto uniform_flat_blue = std::make_shared<UniformTexture>(Color(0, 0, 255));
    auto uniform_flat_cyan = std::make_shared<UniformTexture>(Color(0, 255, 254));
    auto uniform_flat_random = std::make_shared<UniformTexture>(flat_random);
    auto uniform_mat_red = std::make_shared<UniformTexture>(mat_red);
     
    Sphere ball1(uniform_flat_red, Point4(0, 0, 25), 10);
    Sphere ball2(uniform_flat_cyan, Point4(7, -4, 15), 3);
    Sphere ball3(uniform_flat_random, Point4(7, 10, 6), 10);
    Sphere ball4(uniform_mat_red, Point4(-7, 8, 6), 1);

    // Triangle triangle1 = Triangle(Point4(1, 0, 8), Point4(0, 1 * std::sqrt(3), 8), Point4(-1, 0, 8), uniform_flat_random);
    // Triangle triangle2 = Triangle(Point4(14, 5, 6), Point4(6, 7, 8), Point4(20, -1, 4), uniform_mat_red);
    Triangle triangle11 = Triangle(Point4(-10, -15, 10), Point4(-5, -15, 10), Point4(-10, -5, 10), uniform_mat_red);
    Triangle triangle12 = Triangle(Point4(-5, -15, 10), Point4(-5, -5, 10), Point4(-10, -5, 10), uniform_mat_red);
    Triangle triangle13 = Triangle(Point4(-5, -15, 10), Point4(-5, -15, 15), Point4(-5, -5, 10), uniform_flat_random);
    Triangle triangle14 = Triangle(Point4(-5, -15, 15), Point4(-5, -5, 15), Point4(-5, -5, 10), uniform_flat_random);
    Triangle triangle15 = Triangle(Point4(-10, -5, 10), Point4(-5, -5, 10), Point4(-10, -5, 15), uniform_flat_blue);
    Triangle triangle16 = Triangle(Point4(-5, -5, 10), Point4(-5, -5, 15), Point4(-10, -5, 15), uniform_flat_blue);

    Triangle triangle21 = Triangle(Point4(-10, 5, 10), Point4(-5, 5, 10), Point4(-10, 15, 10), uniform_mat_red);
    Triangle triangle22 = Triangle(Point4(-5, 5, 10), Point4(-5, 15, 10), Point4(-10, 15, 10), uniform_mat_red);
    Triangle triangle23 = Triangle(Point4(-5, 5, 10), Point4(-5, 5, 15), Point4(-5, 15, 10), uniform_flat_random);
    Triangle triangle24 = Triangle(Point4(-5, 5, 15), Point4(-5, 15, 15), Point4(-5, 15, 10), uniform_flat_random);
    Triangle triangle25 = Triangle(Point4(-10, 5, 10), Point4(-10, 5, 15), Point4(-5, 5, 10), uniform_flat_blue);
    Triangle triangle26 = Triangle(Point4(-10, 5, 15), Point4(-5, 5, 15), Point4(-5, 5, 10), uniform_flat_blue);
    
    Triangle triangle31 = Triangle(Point4(5, -15, 10), Point4(10, -15, 10), Point4(5, -5, 10), uniform_mat_red);
    Triangle triangle32 = Triangle(Point4(10, -15, 10), Point4(10, -5, 10), Point4(5, -5, 10), uniform_mat_red);
    Triangle triangle33 = Triangle(Point4(5, -15, 10), Point4(5, -5, 10), Point4(5, -15, 15), uniform_flat_random);
    Triangle triangle34 = Triangle(Point4(5, -15, 15), Point4(5, -5, 10), Point4(5, -5, 15), uniform_flat_random);
    Triangle triangle35 = Triangle(Point4(5, -5, 10), Point4(10, -5, 10), Point4(5, -5, 15), uniform_flat_blue);
    Triangle triangle36 = Triangle(Point4(10, -5, 10), Point4(10, -5, 15), Point4(5, -5, 15), uniform_flat_blue);

    Triangle triangle41 = Triangle(Point4(-3, 7, 10), Point4(15, 7, 10), Point4(-3, 13, 10), uniform_mat_red);
    Triangle triangle42 = Triangle(Point4(15, 7, 10), Point4(15, 13, 10), Point4(-3, 13, 10), uniform_mat_red);
    Triangle triangle45 = Triangle(Point4(-3, 7, 10), Point4(-3, 7, 15), Point4(15, 7, 10), uniform_flat_blue);
    Triangle triangle46 = Triangle(Point4(-3, 7, 15), Point4(15, 7, 15), Point4(15, 7, 10), uniform_flat_blue);
    // Complex *obj = new Complex(vec);

    PointLight top_light(Point4(0, 0, 5), 0.8);
    PointLight bot_light(Point4(3, 0, 2), 0.6);
    CircleLight circle_light(Point4(18, -5, 8), 0.6, 3, Point4(-7, 8, 6));

    Camera camera(Point4(0, 0, 0), Vector4(0, 0, 1), Vector4(0, 1, 0), 45, 45, Point4(0, 0, 5));
 
    Scene scene;
    scene.addObject(ball1);
    scene.addObject(ball2);
    scene.addObject(ball3);
    scene.addObject(ball4);
    scene.addObject(triangle11);
    scene.addObject(triangle12);
    scene.addObject(triangle13);
    scene.addObject(triangle14);
    scene.addObject(triangle15);
    scene.addObject(triangle16);

    scene.addObject(triangle21);
    scene.addObject(triangle22);
    scene.addObject(triangle23);
    scene.addObject(triangle24);
    scene.addObject(triangle25);
    scene.addObject(triangle26);

    scene.addObject(triangle31);
    scene.addObject(triangle32);
    scene.addObject(triangle33);
    scene.addObject(triangle34);
    scene.addObject(triangle35);
    scene.addObject(triangle36);

    scene.addObject(triangle41);
    scene.addObject(triangle42);
    scene.addObject(triangle45);
    scene.addObject(triangle46);
    scene.addLight(top_light);
    // scene.addLight(circle_light);
    scene.setCamera(camera);
    time_t load_end = std::time(nullptr);
    std::cout << "Took: " << load_end - load_start << "s to Load objects" << std::endl;

    time_t start = std::time(nullptr);
    auto img = computeScene(scene, 1080, 1080);
    time_t end = std::time(nullptr);
    std::cout << "\nTook: " << end - start << "s" << std::endl;

    std::cout << "Saving Image" << std::endl;
    img.save_image("results/" + std::string(argv[1]));

    return 0;

}
