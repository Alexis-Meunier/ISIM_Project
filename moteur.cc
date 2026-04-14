#include "moteur.hh"

Color cast_ray(const Point4& hit, Vector4& ray, Object* obj, Scene& scene,
               int depth, float previous_eta = 1);

// Angle returned in radians [0, Pi/2]
float get_angle_margin(const TextureInfo& text)
{
    return text.kd * (M_PI / 2.f);
}

Vector4 get_outgoing_ray(float angle_margin, Vector4& normal,
                         Vector4& reflected)
{
    float rand_angle = (static_cast<float>(rand()) / RAND_MAX) * angle_margin;
    float rand_dir = (static_cast<float>(rand()) / RAND_MAX) * 2.f * M_PI;

    auto diff_from_reflected = Vector4{ 0, 1, 0 };
    if (dot_product(normal, reflected) == 1)
    {
        diff_from_reflected = Vector4{ 1, 0, 0 };
    }

    auto u = cross_product(reflected, diff_from_reflected);
    auto v = cross_product(reflected, u);

    // auto new_vec = reflected * std::cos(rand_angle)
    //     + u * (std::sin(rand_angle) * std::cos(rand_dir))
    //     + v * (std::sin(rand_angle) * std::sin(rand_dir));

    // if (dot_product(normal, new_vec) < 0)
    // {
    //     return get_outgoing_ray(angle_margin, normal, reflected);
    // }

    while (true)
    {
        auto new_vec = reflected * std::cos(rand_angle)
            + u * (std::sin(rand_angle) * std::cos(rand_dir))
            + v * (std::sin(rand_angle) * std::sin(rand_dir));

        if (dot_product(normal, new_vec) >= 0)
            return new_vec;
    }

    return Vector4();
}

Color compute_direct_rays(Scene& scene, const Point4& hit, Vector4& normal,
                          Point4& offset_hit, TextureInfo* info)
{
    if (info->kd <= 0.1f)
        return Color(0, 0, 0);

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
        Vector4 to_light(centroid.x - hit.x, centroid.y - hit.y,
                         centroid.z - hit.z);
        float dist_to_light = std::sqrt(dot_product(to_light, to_light));
        to_light.normalize();

        // Compute angle
        float cos_theta = dot_product(normal, to_light);
        if (cos_theta <= 0)
            continue;

        // Shadow check
        auto [sh, so] =
            bvh_intersect(scene.bvh_pool, scene.objects, offset_hit, to_light);
        if (sh && so != light_obj && so->get_texture(*sh)->kr == 0)
        {
            float to_blocker = distance(offset_hit, *sh);
            if (to_blocker < dist_to_light - 0.01f)
                continue; // in shadow
        }

        // Color of light * color of object * power of light * diffuse variable
        // * angle of rays
        float power = li->lightPower * cos_theta * info->kd;
        direct_r += (li->color->colors[RED] / 255.f) * power * ar;
        direct_g += (li->color->colors[GREEN] / 255.f) * power * ag;
        direct_b += (li->color->colors[BLUE] / 255.f) * power * ab;
    }

    return Color(direct_r * 255, direct_g * 255, direct_b * 255);
}

Vector4 get_ingoing_ray(Vector4& dir, Vector4& normal, float eta)
{
    float cos_dir_norm =
        -dot_product(dir, normal); //- as the 2 point in different directions
    float sin_squared_ingoing = eta * eta * (1 - (cos_dir_norm * cos_dir_norm));
    if (sin_squared_ingoing > 1)
        return Vector4(); // error, total internal refraction
    float cos_ingoing = std::sqrt(1 - sin_squared_ingoing);

    auto ingoing_ray = dir * eta + normal * (eta * cos_dir_norm - cos_ingoing);
    // std::cout << "norm = " << normal << "Ingoing = " << ingoing_ray
    //<< std::endl;
    return ingoing_ray;
}

Color compute_indirect_rays(Scene& scene, Object* obj, Vector4& normal,
                            const Point4& hit, Vector4& ray, TextureInfo* info,
                            int depth, float previous_eta = 1)
{
    // Ready needed variables such as angle with BRDF
    float ar = info->color->colors[RED] / 255.f;
    float ag = info->color->colors[GREEN] / 255.f;
    float ab = info->color->colors[BLUE] / 255.f;

    // auto V = -(ray + hit);
    // V.normalize();
    ray.normalize();
    auto for_ref = ray * -1;
    bool one_refracted = false;

    float eta;
    if (dot_product(ray, normal) > 0)
    {
        if (info->eta == 0)
            eta = previous_eta;
        else
            eta = previous_eta / info->eta;
        normal = normal * -1;
    }
    else if (info->eta != 0)
        eta = info->eta / previous_eta;
    else
        eta = previous_eta;

    auto R = normal * 2.0f * dot_product(normal, for_ref) - for_ref;
    R.normalize();

    float indirect_r = 0, indirect_g = 0, indirect_b = 0;
    float margin_angle = get_angle_margin(*info);
    int nb_valid = 0;
    // divide refraction index of where we are coming from
    // where we are going
    float next_eta =
        (info->eta == 0 || dot_product(ray, normal) > 0) ? 1.0f : info->eta;
    // Send rays to get indirect lighting
    for (int i = 0; i < NB_RAYS_REFLECTED; i++)
    {
        float rd = (static_cast<float>(rand()) / RAND_MAX);
        Vector4 new_ray;
        if (rd > (1 - info->kr))
        {
            if (one_refracted)
                continue;
            // std::cout << eta << std::endl;
            new_ray = get_ingoing_ray(ray, normal, eta);
            one_refracted = true;
            /*if (dir.x != new_ray.x || dir.y != new_ray.y || dir.z !=
            new_ray.z)
            {
                std::cout << "dir = " << dir;
                std::cout << "new ray = " << new_ray << std::endl;
            }*/
        }
        else
        {
            new_ray = get_outgoing_ray(margin_angle, normal, R);
            // if (info->kr > 0)
            // std::cout << "rd = " << rd << " kr = " << 1 - info->kr
            //          << std::endl;
        }

        Point4 offset_hit(hit.x + new_ray.x * 0.001f,
                          hit.y + new_ray.y * 0.001f,
                          hit.z + new_ray.z * 0.001f);

        auto [next_hit, next_obj] =
            bvh_intersect(scene.bvh_pool, scene.objects, offset_hit, new_ray);
        // Hit nothing
        if (!next_hit || distance(*next_hit, offset_hit) < 0.001f)
            continue;

        float cos_theta = std::max(0.f, dot_product(normal, new_ray));
        float diffuse_w = info->kd * cos_theta;
        float specular_w = info->ks; // specular doesn't attenuated by cos_theta
        float refracted_w = info->kr;

        float w = diffuse_w + specular_w;

        // Send ray towards the object
        Color incoming =
            cast_ray(*next_hit, new_ray, next_obj, scene, depth + 1, next_eta);
        if (rd > (1 - info->kr))
        {
            indirect_r += (incoming.colors[RED] / 255.f) * refracted_w;
            indirect_g += (incoming.colors[GREEN] / 255.f) * refracted_w;
            indirect_b += (incoming.colors[BLUE] / 255.f) * refracted_w;
        }
        else
        {
            indirect_r += (incoming.colors[RED] / 255.f) * w * ar;
            indirect_g += (incoming.colors[GREEN] / 255.f) * w * ag;
            indirect_b += (incoming.colors[BLUE] / 255.f) * w * ab;
        }
        nb_valid++;
    }

    if (nb_valid > 0)
    {
        indirect_r /= nb_valid;
        indirect_g /= nb_valid;
        indirect_b /= nb_valid;
    }

    return Color(indirect_r * 255, indirect_g * 255, indirect_b * 255);
}

Color compute_light_rays(LightTexture* light_tex, const Point4& hit, int depth)
{
    // Only consider the first depth, further depths will be handled later
    if (depth == 0)
    {
        auto li = light_tex->get_elements(hit);
        return Color(li->color->colors[RED] * li->lightPower,
                     li->color->colors[GREEN] * li->lightPower,
                     li->color->colors[BLUE] * li->lightPower);
    }

    auto li = light_tex->get_elements(hit);
    return Color(li->color->colors[RED] * li->lightPower,
                 li->color->colors[GREEN] * li->lightPower,
                 li->color->colors[BLUE] * li->lightPower);
    // return Color(0, 0, 0);
}

Color cast_ray(const Point4& hit, Vector4& ray, Object* obj, Scene& scene,
               int depth, float previous_eta)
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
    Point4 offset_hit(hit.x + normal.x * 0.001f, hit.y + normal.y * 0.001f,
                      hit.z + normal.z * 0.001f);

    // Compute direct light rays (NEE)
    auto direct_c = compute_direct_rays(scene, hit, normal, offset_hit, info);
    float direct_r = direct_c.colors[RED] / 255.f;
    float direct_g = direct_c.colors[GREEN] / 255.f;
    float direct_b = direct_c.colors[BLUE] / 255.f;

    // Compute indirect colors (recursion)
    auto indirect_c = compute_indirect_rays(scene, obj, normal, hit, ray, info,
                                            depth, previous_eta);

    float indirect_r = indirect_c.colors[RED] / 255.f;
    float indirect_g = indirect_c.colors[GREEN] / 255.f;
    float indirect_b = indirect_c.colors[BLUE] / 255.f;

    return Color(std::min(255.f, (direct_r + indirect_r) * 255.f),
                 std::min(255.f, (direct_g + indirect_g) * 255.f),
                 std::min(255.f, (direct_b + indirect_b) * 255.f));
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
                auto color = cast_ray(*intersection, ray, obj, scene, 0);
                red += color.colors[RED];
                green += color.colors[GREEN];
                blue += color.colors[BLUE];
            }

            // Average values of each ray
            img.pixels[i * image_w + j] =
                Color(red / NB_RAYS, green / NB_RAYS, blue / NB_RAYS);
            printProgressBar(i * image_w + j, image_w * image_h);
        }
    }

    printProgressBar(50, 50);
    return img;
}

int main(int argc, char** argv)
{
    // auto bot_bound = Mesh::rectangle(Point4(-40, -20, 50), Point4(40, -20, 50), Point4(40, -20, 0), Point4(-40, -20, 0), image_texture);
    // auto left_bound = Mesh::rectangle(Point4(-40, 20, -100), Point4(-40, 20, 50), Point4(-40, -20, 50), Point4(-40, -20, -100), uniform_flat_blue);
    // auto right_bound = Mesh::rectangle(Point4(40, 20, 50), Point4(40, 20, -100), Point4(40, -20, -100), Point4(40, -20, 50), uniform_flat_green);
    // auto top_bound = Mesh::rectangle(Point4(-40, 20, -100), Point4(40, 20, -100), Point4(40, 20, 50), Point4(-40, 20, 50), uniform_flat_red);
    // auto forward_bound = Mesh::rectangle(Point4(-100, 20, 35), Point4(100, 20, 35), Point4(100, -20, 35), Point4(-100, -20, 35), uniform_flat_cyan);
    
    // scene.addObject(bot_bound);
    // scene.addObject(top_bound);
    // scene.addObject(right_bound);
    // scene.addObject(left_bound);
    // scene.addObject(forward_bound);

    // time_t load_border = std::time(nullptr);
    // printTimeTaken(load_obj, load_border, "while loading borders.");

    if (argc != 2)
    {
        std::cerr << "Usage: ./tp1 <filename>" << std::endl;
        return 1;
    }

    // time_t load_start = std::time(nullptr);

    // TextureInfo mat_red{ kd: 0.8f, ks: 0.2, color: new Color(255, 0, 0) };
    // LightInfo light;
    // light.kd = 0.5f;
    // light.ks = 0.2f;
    // light.color = new Color(255, 255, 255);
    // light.lightPower = 0.8f;

    // time_t load_textureInfo = std::time(nullptr);
    // printTimeTaken(load_start, load_textureInfo, "while loading texture Infos.");

    // auto uniform_flat_red = std::make_shared<UniformTexture>(Color(182, 35, 48));
    // auto uniform_mat_red = std::make_shared<UniformTexture>(&mat_red);
    // auto light_texture = std::make_shared<LightTexture>(&light);

    // time_t load_textures = std::time(nullptr);
    // printTimeTaken(load_textureInfo, load_textures, "while loading textures.");

    // Sphere light_ball(light_texture, Point4(0, 20, 40), 3);
    // Sphere light_ball2(light_texture, Point4(0, 0, -10), 3);

    // time_t load_spheres = std::time(nullptr);
    // printTimeTaken(load_textures, load_spheres, "while loading spheres.");
    
    // Vector4 looking_at(0, 0, 1);
    // looking_at.normalize();
    // Camera camera(Point4(0, 0, 0), looking_at, Vector4(0, 1, 0), 45, 45,
    //               Point4(0, 0, 5));

    // Scene scene;
    // scene.addLights(light_ball);
    // scene.addLights(light_ball2);
    // scene.setCamera(camera);

    // time_t start = std::time(nullptr);
    // auto img = computeScene(scene, 1080, 1080);
    // time_t end = std::time(nullptr);
    // printTimeTaken(start, end, "");

    // std::cout << "Saving Image" << std::endl;
    // img.save_image("results/" + std::string(argv[1]) + ".ppm");

    auto im = createRandomImage(500, 500, 5, 0.2, 2, 50);
    im.save_image("results/random.ppm");

    auto wood = createWoodTexture(500, 500, 6, 0.7, 2.0, 120);
    wood.save_image("results/wood.ppm");

    auto cloud = createCloudTexture(500, 500, 4, 0.6, 2.0, 200);
    cloud.save_image("results/cloud.ppm");

    return 0;
}
