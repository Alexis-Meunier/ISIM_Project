#include "utils/scene.hh"

#include <algorithm>
#include <cmath>
#include <random>
#include <memory>
#include <ctime>

#include "image/image.hh"
#include "objects/sphere.hh"
#include "objects/triangle.hh"
// #include "objects/complex.hh"
#include "light/point_light.hh"
#include "light/circle_light.hh"

#define NB_RAYS 1

int val = -1;

/**
 * Returns the euclidian norm/Distance between two points
 * 
 * Reminder:                    
 *      In R^3:
 *          v1 = (x1, y1, z1)
 *          v2 = (x2, y2, z2) 
 * 
 * ||v1 - v2|| = sqrt(
 *      (x1 - x2)^2
 *    + (y1 - y2)^2
 *    + (z1 - z2)^2
 * )
 */
float distance(const Point4& p1, const Point4& p2)
{
    // Norm computation
    return std::sqrt(
        std::pow(p1.x - p2.x, 2) +
        std::pow(p1.y - p2.y, 2) +
        std::pow(p1.z - p2.z, 2)
    );
}

/**
 * Returns the result of the cross product between v1 and v2
 * 
 * Reminder:
 *      In R^3:
 *          v1 = (x1, y1, z1)
 *          v2 = (x2, y2, z2) 
 *
 * ==>  | i  j  k  |
 *      | x1 y1 z1 |
 *      | x2 y2 z2 |
 * 
 * v1 x v2 = 
 *      i * (y1 * z2 - z1 * y2),
 *    - j * (x1 * z2 - x2 * z1),
 *    + k * (x1 * y2 - x2 * y1)
*/
Vector4 cross_product(const Vector4& v1, const Vector4& v2)
{
    return Vector4(
        v1.y * v2.z - v1.z * v2.y,
        -(v1.x * v2.z - v1.z * v2.x),
        v1.x * v2.y - v1.y * v2.x
    );
}

/**
 * Computes the dot product between two vector
 * 
 * Reminder:
 *      In R^3:
 *          v1 = (x1, y1, z1)
 *          v2 = (x2, y2, z2)
 * 
 * <v1, v2> = x1*x2 + y1*y2 + z1*z2
 * 
 * N.B.: Here the function calls the overrident * operator
 *       and clamps it to zero
 */
float dot_product(const Vector4& light, const Vector4& point)
{
    float val = light * point;
    if (val < 0)
        return 0;
    return val;
}

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
float compute_diffuse_light(const float& kd, const float& color, float& lightPower, float& dotProduct)
{
    lightPower = lightPower > 1 ? 1 : lightPower;
    dotProduct = dotProduct > 1 ? 1 : dotProduct;

    auto val = kd * color * lightPower * dotProduct;
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
float compute_specular_light(const TextureInfo& info, const float& color, float& lightPower, float& dotProduct)
{
    lightPower = lightPower > 1 ? 1 : lightPower;
    dotProduct = dotProduct > 1 ? 1 : dotProduct;

    auto val = info.ks * color * lightPower * std::pow(dotProduct, info.ns);
    return val > 1 ? 1 : val;
}


std::pair<std::optional<Point4>, Object*> bvh_intersect(
    const std::vector<BVHNode>& pool,
    const std::vector<Object*>& objs,
    const Point4& origin,
    const Vector4& dir,
    int node_idx = 0)
{
    const BVHNode& node = pool[node_idx];

    if (!node.bounds.intersect(origin, dir))
        return {std::nullopt, nullptr};

    if (node.is_leaf()) {
        // Linear search within the leaf (at most 4 objects for now)
        float best_dist = std::numeric_limits<float>::max();
        std::optional<Point4> best_hit = std::nullopt;
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

Color compute_color(const Point4& hit, Object* obj, const Scene& scene, int depth)
{
    if (depth == 5)
        return Color();

    // Instantiate all variables we need to compute the color of the pixels
    TextureInfo info = obj->get_texture(hit);
    Color *color = info.color;
    float kd = info.kd;
    float li = 0;
    float li_red = 0;
    float li_blue = 0;
    float li_green = 0;
    float nl = 0;
    float products = 0;
    float contribution_red = 0;
    float contribution_green = 0;
    float contribution_blue = 0;

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
        li += light->power;
        li_red += light->power * r;
        li_blue += light->power * g;
        li_green += light->power * b;
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
        for (const auto& sample : lightSamples)
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

        nl += visibility * (nl_local / nSamples);
        products += visibility * (spec_local / nSamples);
    }

    // Recursively call the function for other objects contribution
    auto [refl_hit, refl_obj] = bvh_intersect(scene.bvh_pool, scene.objects, hit, R);
    if (refl_hit && refl_obj != obj)
    {
        auto new_color = compute_color(*refl_hit, refl_obj, scene, depth + 1);
        contribution_red   += info.ks * new_color.colors[RED] / 255.;
        contribution_green += info.ks * new_color.colors[GREEN] / 255.;
        contribution_blue  += info.ks * new_color.colors[BLUE] / 255.;
    }

    // Clamp values
    li = li > 1 ? 1 : li;
    // std::cout << "li_red: " << li_red << std::endl;
    // std::cout << "li_blue: " << li_blue << std::endl;
    // std::cout << "li_green: " << li_green << std::endl;
    nl = nl > 1 ? 1 : nl;

    // Compute diffuse and specular contribution for each color channel
    auto diff_red = compute_diffuse_light(kd, (color->colors[RED] / 255.), li, nl);
    auto diff_green = compute_diffuse_light(kd, (color->colors[GREEN] / 255.), li, nl);
    auto diff_blue = compute_diffuse_light(kd, (color->colors[BLUE] / 255.), li, nl);

    // DEBUGGING
    // if (val == 4)
    // {
    //     std::cout << "object nb." << val << "\n\t- diff_red: " << diff_red
    //             << "\n\t- diff_green: " << diff_green
    //             << "\n\t- diff_blue: " << diff_blue << std::endl; 
    //     std::cout << "R: " << int(color->colors[RED]) << std::endl;
    //     std::cout << "G: " << int(color->colors[GREEN]) << std::endl;
    //     std::cout << "B: " << int(color->colors[BLUE]) << std::endl;
    //     std::cout << "kd: " << kd << std::endl;
    //     std::cout << "li: " << li << std::endl;
    //     std::cout << "nl: " << nl << std::endl;
    // }

    auto spec_red = compute_specular_light(info, (color->colors[RED] / 255.), li, products);
    auto spec_green = compute_specular_light(info, (color->colors[GREEN] / 255.), li, products);
    auto spec_blue = compute_specular_light(info, (color->colors[BLUE] / 255.), li, products);

    auto red = std::min(1.0f, diff_red + spec_red + li_red * nl);
    auto green = std::min(1.0f, diff_green + spec_green + li_green * nl);
    auto blue = std::min(1.0f, diff_blue + spec_blue + li_blue * nl);

    return Color(255 * red, 255 * green, 255 * blue);
}

std::pair<std::optional<Point4>, Object*> find_closest_intersection(const Point4& ray_origin, const Vector4& ray_dir, const std::vector<Object*>& objects)
{
    float closest_dist = std::numeric_limits<float>::max();
    std::optional<Point4> closest_hit = std::nullopt;
    Object *closest_obj = nullptr;

    int index = 0;
    for (auto obj : objects)
    {
        auto hit = obj->intersect(ray_origin, ray_dir);
        if (hit != std::nullopt)
        {
            float dist = distance(ray_origin, *hit);
            if (dist < closest_dist)
            {
                closest_dist = dist;
                closest_hit = hit;
                closest_obj = obj;
                val = index;
            }
        }
        index++;
    }

    return {closest_hit, closest_obj};
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

    time_t start = std::time(nullptr);
    scene.build_bvh();
    time_t end = std::time(nullptr);
    std::cout << "Took " << end - start << "s to build the BVH\n";

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
                auto p = compute_color(*intersection, obj, scene, 0);
                red += p.colors[RED];
                green += p.colors[GREEN];
                blue += p.colors[BLUE];
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

    PointLight top_light(Point4(0, 0, 5), 0.8, Color(255, 255, 255));
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
