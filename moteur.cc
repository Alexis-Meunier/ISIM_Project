#include "moteur.hh"

Color cast_ray(const Point4& hit, Object* obj, Scene& scene, int depth);


// Angle returned in radians [0, Pi/2]
float get_angle_margin(const TextureInfo& text)
{
    return (1.f - text.ks) * (M_PI / 2.f);
}

Vector4 get_outgoing_ray(float angle_margin, Vector4& normal, Vector4& reflected)
{
    float rand_angle = (static_cast<float>(rand()) / RAND_MAX) * angle_margin;
    float rand_dir   = (static_cast<float>(rand()) / RAND_MAX) * 2.f * M_PI;

    auto diff_from_reflected = Vector4{ 0, 1, 0 };
    if (dot_product(normal, reflected) == 1)
    {
        diff_from_reflected = Vector4{ 1, 0, 0 };
    }

    auto u = cross_product(reflected, diff_from_reflected);
    auto v = cross_product(reflected, u);

    auto new_vec = reflected * std::cos(rand_angle)
        + u * (std::sin(rand_angle) * std::cos(rand_dir))
        + v * (std::sin(rand_angle) * std::sin(rand_dir));

    if (dot_product(normal, new_vec) < 0)
    {
        return get_outgoing_ray(angle_margin, normal, reflected);
    }

    return new_vec;
}

Color compute_direct_rays(Scene& scene, const Point4& hit, Vector4& normal, Point4& offset_hit, TextureInfo *info)
{
    if (info->kd <= 0.1f) return Color(0, 0, 0);

    float direct_r = 0, direct_g = 0, direct_b = 0;
    float ar = info->color->colors[RED] / 255.f;
    float ag = info->color->colors[GREEN] / 255.f;
    float ab = info->color->colors[BLUE] / 255.f;

    // Use NEE to always have at least one light (if possible)
    for (Object* light_obj : scene.lights)
    {
        // Lights array only has lightTexture
        auto ltex = dynamic_cast<LightTexture*>(light_obj->texture.get());
        auto li = ltex->get_elements(hit);

        // Get the center of the light
        auto centroid = light_obj->get_centroid();
        Vector4 to_light(centroid.x - hit.x,
                         centroid.y - hit.y,
                         centroid.z - hit.z);
        float dist_to_light = std::sqrt(dot_product(to_light, to_light));
        to_light.normalize();

        // Compute angle
        float cos_theta = dot_product(normal, to_light);
        if (cos_theta <= 0) continue;
        
        // Shadow check
        auto [sh, so] = bvh_intersect(scene.bvh_pool, scene.objects, offset_hit, to_light);
        if (sh && so != light_obj) {
            float to_blocker = distance(offset_hit, *sh);
            if (to_blocker < dist_to_light - 0.01f) continue; // in shadow
        }

        // Color of light * color of object * power of light * diffuse variable * angle of rays
        float power = li->lightPower * cos_theta * info->kd;
        direct_r += (li->color->colors[RED] / 255.f) * power * ar;
        direct_g += (li->color->colors[GREEN] / 255.f) * power * ag;
        direct_b += (li->color->colors[BLUE] / 255.f) * power * ab;
    }

    return Color(direct_r * 255, direct_g * 255, direct_b * 255);
}

Color compute_indirect_rays(Scene& scene, Object *obj, Vector4& normal, const Point4& hit, Point4& offset_hit, TextureInfo *info, int depth)
{
    // Ready needed variables such as angle with BRDF
    float ar = info->color->colors[RED] / 255.f;
    float ag = info->color->colors[GREEN] / 255.f;
    float ab = info->color->colors[BLUE] / 255.f;

    auto V = Vector4(scene.camera.center - hit);
    V.normalize();

    auto R = normal * 2.0f * dot_product(normal, V) - V;
    R.normalize();

    float indirect_r = 0, indirect_g = 0, indirect_b = 0;
    float margin_angle = get_angle_margin(*info);
    int nb_valid = 0;

    // Send rays to get indirect lighting
    for (int i = 0; i < NB_RAYS_REFLECTED; i++)
    {
        Vector4 outgoing = get_outgoing_ray(margin_angle, normal, R);

        auto [next_hit, next_obj] = bvh_intersect(scene.bvh_pool, scene.objects, offset_hit, outgoing);
        // Hit nothing
        if (!next_hit || next_obj == obj) continue;


        float cos_theta = std::max(0.f, dot_product(normal, outgoing));
        float diffuse_w  = info->kd * cos_theta;
        float specular_w = info->ks; // specular doesn't attenuate by cos_theta

        float w = diffuse_w + specular_w;

        // Send ray towards the object
        Color incoming = cast_ray(*next_hit, next_obj, scene, depth + 1);
        indirect_r += (incoming.colors[RED] / 255.f) * w * ar;
        indirect_g += (incoming.colors[GREEN] / 255.f) * w * ag;
        indirect_b += (incoming.colors[BLUE] / 255.f) * w * ab;
        nb_valid++;
    }

    if (nb_valid > 0) {
        indirect_r /= nb_valid;
        indirect_g /= nb_valid;
        indirect_b /= nb_valid;
    }

    return Color(indirect_r * 255, indirect_g * 255, indirect_b * 255);
}

Color compute_light_rays(LightTexture *light_tex, const Point4& hit, int depth)
{
    // Only consider the first depth, further depths will be handled later
    if (depth == 0)
    {
        auto li = light_tex->get_elements(hit);
        return Color(
            li->color->colors[RED] * li->lightPower,
            li->color->colors[GREEN] * li->lightPower,
            li->color->colors[BLUE] * li->lightPower
        );
    }

    return Color(0, 0, 0);
}

Color cast_ray(const Point4& hit, Object* obj, Scene& scene, int depth)
{
    if (depth == MAX_DEPTH)
        return Color(0, 0, 0);

    auto light_tex = dynamic_cast<LightTexture*>(obj->texture.get());
    // The object that was hit is a light
    if (light_tex != nullptr)
    {
        return compute_light_rays(light_tex, hit, depth);
    }

    TextureInfo* info = obj->get_texture(hit);
    auto normal = obj->get_normal(hit);
    normal.normalize();

    // Offset in case of self-intersection
    Point4 offset_hit(
        hit.x + normal.x * 0.001f,
        hit.y + normal.y * 0.001f,
        hit.z + normal.z * 0.001f
    );

    // Compute direct light rays (NEE)
    auto direct_c = compute_direct_rays(scene, hit, normal, offset_hit, info);
    float direct_r = direct_c.colors[RED] / 255.f;
    float direct_g = direct_c.colors[GREEN] / 255.f;
    float direct_b = direct_c.colors[BLUE] / 255.f;

    // Compute indirect colors (recursion)
    auto indirect_c = compute_indirect_rays(scene, obj, normal, hit, offset_hit, info, depth);
    float indirect_r = indirect_c.colors[RED] / 255.f;
    float indirect_g = indirect_c.colors[GREEN] / 255.f;
    float indirect_b = indirect_c.colors[BLUE] / 255.f;

    return Color(
        std::min(255.f, (direct_r + indirect_r) * 255.f),
        std::min(255.f, (direct_g + indirect_g) * 255.f),
        std::min(255.f, (direct_b + indirect_b) * 255.f)
    );
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
                float r1 = -0.5f * x_incr
                    + (static_cast<float>(rand()) / RAND_MAX) * x_incr;
                float r2 = -0.5f * y_incr
                    + (static_cast<float>(rand()) / RAND_MAX) * y_incr;

                // Get pixel position and ray sent through it
                Point4 pixel(
                    cam.zmin.x + (x + r1) * horizontal.x + (y + r2) * real_up.x,
                    cam.zmin.y + (x + r1) * horizontal.y + (y + r2) * real_up.y,
                    cam.zmin.z + (x + r1) * horizontal.z
                        + (y + r2) * real_up.z);
                Vector4 ray(pixel.x - cam.center.x, pixel.y - cam.center.y,
                            pixel.z - cam.center.z);
                ray.normalize();

                // Find first hit object
                auto [intersection, obj] = bvh_intersect(
                    scene.bvh_pool, scene.objects, cam.center, ray);
                if (intersection == std::nullopt)
                    continue;

                // Compute its color and add it to the pixel color
                auto color = cast_ray(*intersection, obj, scene, 0);
                red += color.colors[RED];
                green += color.colors[GREEN];
                blue += color.colors[BLUE];
            }

            // Average values of each ray
            img.pixels[i * image_w + j] =
                new Color(red / NB_RAYS, green / NB_RAYS, blue / NB_RAYS);
            printProgressBar(i * image_w + j, image_w * image_h);
        }
    }

    printProgressBar(50, 50);
    return img;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./tp1 <filename>" << std::endl;
        return 1;
    }

    time_t load_start = std::time(nullptr);
    TextureInfo
    flat_random{ kd: 0.4, ks: 0.2, ns: 0.5, color: new Color(68, 164, 112) };
    TextureInfo
    a{ kd: 0.2f, ks: 0.8f, ns: 0.9, color: new Color(255, 255, 0) };
    TextureInfo
    mat_red{ kd: 0.8f, ks: 0.2, ns: 0.9, color: new Color(255, 0, 0) };
    TextureInfo
    something_text{ kd: 0.3, ks: 0.3, ns: 0.9, color: new Color(0, 255, 255) };

    LightInfo light;
    light.kd = 0.5f;
    light.ks = 0.2f;
    light.ns = 0.9f;
    light.color = new Color(245,241,184); //#F5F1B8
    light.lightPower = 0.8f;

    // auto uniform_flat_white = std::make_shared<UniformTexture>(Color(255, 255, 255));
    auto uniform_flat_red = std::make_shared<UniformTexture>(Color(182, 35, 48));
    auto uniform_flat_blue = std::make_shared<UniformTexture>(Color(82, 41, 214));
    auto uniform_flat_green = std::make_shared<UniformTexture>(Color(83, 172, 89));
    auto uniform_flat_cyan = std::make_shared<UniformTexture>(Color(94, 154, 161)); //5e9aa1
    auto uniform_mat_red = std::make_shared<UniformTexture>(&mat_red);
    auto light_texture = std::make_shared<LightTexture>(&light);

    // Sphere ball1(uniform_mat_red, Point4(5, -5, 15), 4);
    Sphere light_ball(light_texture, Point4(0, 20, 20), 3);
    Sphere light_ball2(light_texture, Point4(0, 0, -10), 3);

    auto image_texture = std::make_shared<ImageTexture>("image.jpg");
    // auto mesh = Mesh::rectangle(Point4(-10, -15, 10), Point4(-5, -15, 10), Point4(-5, -5, 10), Point4(-10, -5, 10), image_texture);

    auto car = Mesh::from_obj("plant.obj", uniform_mat_red, Vector4(0, -20, 20), 1.7);
    auto skull = Mesh::from_obj("skull.obj", uniform_mat_red, Vector4(10, -20, 20), 0.7);

    auto bot_bound = Mesh::rectangle(Point4(-40, -20, 0), Point4(40, -20, 0), Point4(40, -20, 50), Point4(-40, -20, 50), image_texture);
    auto left_bound = Mesh::rectangle(Point4(-40, -20, -100), Point4(-40, -20, 50), Point4(-40, 20, 50), Point4(-40, 20, -100), uniform_flat_blue);
    auto right_bound = Mesh::rectangle(Point4(40, -20, 50), Point4(40, -20, -100), Point4(40, 20, -100), Point4(40, 20, 50), uniform_flat_green);
    auto top_bound = Mesh::rectangle(Point4(-40, 20, 50), Point4(40, 20, 50), Point4(40, 20, -100), Point4(-40, 20, -100), uniform_flat_red);
    auto forward_bound = Mesh::rectangle(Point4(-100, -20, 35), Point4(100, -20, 35), Point4(100, 20, 35), Point4(-100, 20, 35), uniform_flat_cyan);

    Camera camera(Point4(0, 0, 0), Vector4(0, 0, 1), Vector4(0, 1, 0), 45, 45,
                  Point4(0, 0, 5));

    Scene scene;
    scene.addObject(bot_bound);
    scene.addObject(top_bound);
    scene.addObject(right_bound);
    scene.addObject(left_bound);
    scene.addObject(forward_bound);

    scene.addObject(car);
    scene.addObject(skull);

    scene.addObject(light_ball);
    scene.addLights(light_ball);
    scene.addObject(light_ball2);
    scene.addLights(light_ball2);
    scene.setCamera(camera);
    time_t load_end = std::time(nullptr);
    std::cout << "Took: " << load_end - load_start << "s to Load objects"
              << std::endl;

    time_t start = std::time(nullptr);
    auto img = computeScene(scene, 1080, 1080);
    time_t end = std::time(nullptr);
    std::cout << "\nTook: " << end - start << "s" << std::endl;

    std::cout << "Saving Image" << std::endl;
    img.save_image("results/" + std::string(argv[1]));

    return 0;
}
