#include "moteur.hh"

#include "utils/vector4.hh"

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

        if ((normal * new_vec) >= 0)
            return new_vec;
        rand_angle = (static_cast<float>(rand()) / RAND_MAX) * angle_margin;
        rand_dir = (static_cast<float>(rand()) / RAND_MAX) * 2.f * M_PI;
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
        // if (!ltex)
        // {
        //     std::cout << "not light\n";
        //     continue;
        // }
        // else
        //     std::cout << "Yes light\n";
        auto li = light_obj->texture->get_elements(hit);

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
        if (sh && so != light_obj && !so->get_texture(*sh)->kr)
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

Vector4 get_ingoing_ray(Vector4& dir, Vector4& normal, float etai, float etat)
{
    float cos_dir_norm = std::clamp(dir * normal, -1.0f, 1.0f);
    auto n = normal;
    if (cos_dir_norm < 0)
    {
        cos_dir_norm = -cos_dir_norm;
    }
    else
    {
        std::swap(etai, etat);
        n = normal * -1;
    }

    float ratio = etai / etat;
    float sin_squared_ingoing =
        ratio * ratio * (1 - (cos_dir_norm * cos_dir_norm));
    if (sin_squared_ingoing > 1)
        return Vector4(); // error, total internal refraction
    float cos_ingoing = std::sqrt(1 - sin_squared_ingoing);

    auto ingoing_ray =
        dir * ratio + normal * (ratio * cos_dir_norm - cos_ingoing);
    ingoing_ray.normalize();
    return ingoing_ray;
}

float fresnel(float cosi, float etai, float etat)
{
    float r0 = (etai - etat) / (etai + etat);
    r0 = r0 * r0;
    float to_ret = r0 + (1 - r0) * std::pow(1 - cosi, 5);
    return to_ret;
}

Color till_cast_ray(Scene& scene, Point4& offset_hit, Vector4& ray,
                    Vector4& normal, TextureInfo* info, int depth,
                    float next_eta)
{
    auto [next_hit, next_obj] =
        bvh_intersect(scene.bvh_pool, scene.objects, offset_hit, ray);
    // Hit nothing

    if (!next_hit || distance(*next_hit, offset_hit) < 0.001f)
        return Color();

    // Send ray towards the object
    return cast_ray(*next_hit, ray, next_obj, scene, depth + 1, next_eta);
}

Color compute_indirect_rays(Scene& scene, Object* obj, Vector4& normal,
                            const Point4& hit, Vector4& ray, TextureInfo* info,
                            int depth, float previous_eta = 1)
{
    // Ready needed variables such as angle with BRDF
    float ar = info->color->colors[RED] / 255.f;
    float ag = info->color->colors[GREEN] / 255.f;
    float ab = info->color->colors[BLUE] / 255.f;

    normal.normalize();

    // auto V = -(ray + hit);
    // V.normalize();
    ray.normalize();

    float next_eta = previous_eta;
    float etai = previous_eta;
    float etat = info->eta;

    if (info->kr)
    {
        if ((ray * normal) > 0)
        {
            normal = normal * -1;
            etat = 1.0f;
        }
    }
    auto for_ref = ray * -1;
    auto R = normal * 2.0f * dot_product(normal, for_ref) - for_ref;
    R.normalize();

    float indirect_r = 0, indirect_g = 0, indirect_b = 0;
    float margin_angle = get_angle_margin(*info);
    int nb_valid = 0;

    // divide refraction index of where we are coming from
    // where we are going
    // Send rays to get indirect lighting
    float rd = (static_cast<float>(rand()) / RAND_MAX);
    float cos_dir_norm = std::min(std::abs(ray * normal), 1.0f);
    float kr = 1;
    if (info->kr)
    {
        kr = fresnel(cos_dir_norm, etai, etat);
        // std::cout << "kr = " << kr << "\nrd = " << rd << "\n";
        next_eta = etat;
    }

    uint8_t nb_ray = NB_RAYS_REFLECTED;
    if (info->ks == 1)
        nb_ray = 1;

    Vector4 ingoing;
    Vector4 outgoing;
    for (int i = 0; i < nb_ray; i++)
    {
        if (info->kr)
            ingoing = get_ingoing_ray(ray, normal, etai, etat);
        outgoing = get_outgoing_ray(margin_angle, normal, R);

        Point4 offset_hit_ingoing;
        Point4 offset_hit_outgoing;
        Color c_ingoing;
        Color c_outgoing;
        if (ingoing * ingoing > 0)
        {
            offset_hit_ingoing =
                Point4(hit.x - normal.x * 0.001f, hit.y - normal.y * 0.001f,
                       hit.z - normal.z * 0.001f);
            c_ingoing = till_cast_ray(scene, offset_hit_ingoing, ingoing,
                                      normal, info, depth, next_eta);
            indirect_r += (c_ingoing.colors[RED] / 255.f) * ar * (1 - kr);
            indirect_g += (c_ingoing.colors[GREEN] / 255.f) * ag * (1 - kr);
            indirect_b += (c_ingoing.colors[BLUE] / 255.f) * ab * (1 - kr);
        }
        if (outgoing * outgoing > 0)
        {
            offset_hit_outgoing =
                Point4(hit.x + normal.x * 0.001f, hit.y + normal.y * 0.001f,
                       hit.z + normal.z * 0.001f);
            c_outgoing = till_cast_ray(scene, offset_hit_outgoing, outgoing,
                                       normal, info, depth, next_eta);

            float cos_theta = dot_product(normal, ray);
            float diffuse_w = info->kd * cos_theta;
            float specular_w =
                info->ks; // specular doesn't get attenuated by cos_theta

            float w = diffuse_w + specular_w;

            // std::cout << c_outgoing;
            indirect_r += (c_outgoing.colors[RED] / 255.f) * w * ar * kr;
            indirect_g += (c_outgoing.colors[GREEN] / 255.f) * w * ag * kr;
            indirect_b += (c_outgoing.colors[BLUE] / 255.f) * w * ab * kr;
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

Color compute_light_rays(std::shared_ptr<TextureMaterial>& light_tex,
                         const Point4& hit, int depth)
{
    // Only consider the first depth, further depths will be handled later
    auto li = light_tex->get_elements(hit);
    return Color(li->color->colors[RED] * li->lightPower,
                 li->color->colors[GREEN] * li->lightPower,
                 li->color->colors[BLUE] * li->lightPower);
}

Color cast_ray(const Point4& hit, Vector4& ray, Object* obj, Scene& scene,
               int depth, float previous_eta)
{
    if (depth == MAX_DEPTH)
        return Color(0, 0, 0);

    // The object that was hit is a light
    if (obj->texture->isLight)
    {
        return compute_light_rays(obj->texture, hit, depth);
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
            // std::cout << " i = " << i << " j = " << j << "\n";
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
    // time_t load_border = std::time(nullptr);
    // printTimeTaken(load_obj, load_border, "while loading borders.");

    if (argc != 2)
    {
        std::cerr << "Usage: ./main <filename>" << std::endl;
        return 1;
    }

    time_t load_start = std::time(nullptr);

    TextureInfo flat_random{ kd: 0.4, ks: 0.2, color: new Color(68, 164, 112) };
    TextureInfo a{ kd: 0.1f, ks: 0.9f, color: new Color(255, 255, 0) };
    TextureInfo mat_red{ kd: 0.6f, ks: 0.4, color: new Color(255, 0, 0) };
    TextureInfo
    something_text{ kd: 0.3, ks: 0.3, color: new Color(0, 255, 255) };
    TextureInfo light;
    light.kd = 0.5f;
    light.ks = 0.2f;
    light.color = new Color(255, 255, 255);
    light.lightPower = 0.8f;
    TextureInfo stronk_light;
    stronk_light.kd = 0.5f;
    stronk_light.ks = 0.2f;
    stronk_light.color = new Color(255, 255, 255);
    stronk_light.lightPower = 0.9f;

    time_t load_textureInfo = std::time(nullptr);
    printTimeTaken(load_start, load_textureInfo,
                   "while loading texture Infos.");

    // auto uniform_flat_white = std::make_shared<UniformTexture>(Color(255,
    // 255, 255));
    auto new_text = std::make_shared<UniformTexture>(Color{ 255, 255, 255 }, 0,
                                                     1, true, 1.5);
    auto new_textr = std::make_shared<UniformTexture>(Color{ 255, 255, 255 }, 0,
                                                      1, false, 1.5);

    auto sky = std::make_shared<ProceduralTexture>(ProceduralType::CLOUD, 1, 0);
    auto sky_light = std::make_shared<ProceduralTexture>(ProceduralType::CLOUD,
                                                         1, 0, 0.8, true);
    auto wood = std::make_shared<ProceduralTexture>(ProceduralType::WOOD, 1, 0);
    auto uniform_flat_blue =
        std::make_shared<UniformTexture>(Color(0, 100, 255));
    auto uniform_flat_green =
        std::make_shared<UniformTexture>(Color(100, 255, 37));
    auto uniform_flat_cyan =
        std::make_shared<UniformTexture>(Color(0, 255, 255));
    auto uniform_mat_red = std::make_shared<UniformTexture>(&mat_red);
    auto light_texture = std::make_shared<LightTexture>(&light);
    auto stronk_light_texture = std::make_shared<LightTexture>(&stronk_light);
    auto image_texture = std::make_shared<ImageTexture>("image.jpg");

    time_t load_textures = std::time(nullptr);
    printTimeTaken(load_textureInfo, load_textures, "while loading textures.");

    auto skull = Mesh::from_obj("skull.obj", uniform_mat_red,
                                Point4(20, -20, 35), 0.7, { -90, 155, 0 });

    // Sphere ball1(uniform_mat_red, Point4(5, -5, 15), 4);
    Sphere light_ball(light_texture, Point4(0, 7, 0), 3);
    Sphere light_ball2(light_texture, Point4(0, 0, -10), 3);
    // Sphere sky_ball(sky, Point4(-15, 0, 30), 10);
    // Sphere wood_ball(wood, Point4(15, 0, 30), 10);

    time_t load_spheres = std::time(nullptr);
    printTimeTaken(load_textures, load_spheres, "while loading spheres.");
    auto new_text_triangle =
        std::make_shared<UniformTexture>(Color{ 255, 255, 0 });
    auto new_text_triangle1 =
        std::make_shared<UniformTexture>(Color{ 0, 255, 100 });
    auto new_text_triangle2 =
        std::make_shared<UniformTexture>(Color{ 255, 50, 150 });

    Sphere ball1(new_text, Point4(0, -4, 9), 3);
    Sphere ball2(uniform_mat_red, Point4(-5, 0, 12), 2);
    Sphere ball3(new_textr, Point4(5, 1, 11), 0.5);
    Sphere light_ball1(light_texture, Point4(-20, 10, 10), 4);
    // Sphere light_ball(light_texture, Point4(0, 15, 0), 4);
    // Sphere light_ball1(light_texture, Point4(-20, 10, 10), 4);

    Triangle triangle1 = Triangle(Point4(-200, -200, 30), Point4(200, -200, 30),
                                  Point4(0, 200, 30), new_text_triangle);
    Triangle triangle2 = Triangle(Point4(0, -10, -100), Point4(200, -10, 200),
                                  Point4(-200, -10, 200), new_text_triangle1);
    Triangle triangle3 = Triangle(Point4(-200, 20, 200), Point4(200, 20, 200),
                                  Point4(0, 20, -100), new_text_triangle2);

    // Complex *obj = new Complex(vec);

    // PointLight top_light(Point4(0, 0, 5), 0.8);
    // PointLight bot_light(Point4(3, 0, 2), 0.6);
    // CircleLight circle_light(Point4(18, -5, 8), 0.6, 3, Point4(-7, 8, 6));

    /*auto car =
        Mesh::from_obj("plant.obj", uniform_mat_red, Point4(0, -20, 20), 1.7);
    auto skull = Mesh::from_obj("skull.obj", uniform_mat_red,
                                Point4(20, -20, 35), 0.7, { -90, 155, 0 });*/

    time_t load_obj = std::time(nullptr);
    printTimeTaken(load_spheres, load_obj, "while loading .obj files.");

    // auto a_bound = Mesh::rectangle(Point4(-25, -5, 50), Point4(-5, -5, 50),
    // Point4(-5, -5, 25), Point4(-25, -5, 25), wood); auto b_bound =
    // Mesh::rectangle(Point4(-25, -5, 25), Point4(-5, -5, 25), Point4(-5, -20,
    // 25), Point4(-25, -20, 25), wood); auto c_bound =
    // Mesh::rectangle(Point4(-5, -5, 25), Point4(-5, -5, 50), Point4(-5, -20,
    // 50), Point4(-5, -20, 25), wood);
    auto bot_bound =
        Mesh::rectangle(Point4(-40, -20, 50), Point4(40, -20, 50),
                        Point4(40, -20, 0), Point4(-40, -20, 0), image_texture);
    auto left_bound = Mesh::rectangle(
        Point4(-40, 20, -100), Point4(-40, 20, 50), Point4(-40, -20, 50),
        Point4(-40, -20, -100), uniform_flat_cyan);
    auto right_bound = Mesh::rectangle(Point4(40, 20, 50), Point4(40, 20, -100),
                                       Point4(40, -20, -100),
                                       Point4(40, -20, 50), uniform_flat_blue);
    auto top_bound =
        Mesh::rectangle(Point4(-40, 20, -100), Point4(40, 20, -100),
                        Point4(40, 20, 50), Point4(-40, 20, 50), sky_light);
    auto forward_bound = Mesh::rectangle(
        Point4(-100, 20, 35), Point4(100, 20, 35), Point4(100, -20, 35),
        Point4(-100, -20, 35), uniform_flat_green);
    auto table = Mesh::cube(Point4(-20, -5, 35), Point4(-5, -5, 35),
                            Point4(-5, -5, 15), Point4(-20, -5, 15), 5, wood);

    Vector4 looking_at(0, 0, 1);
    looking_at.normalize();
    Camera camera(Point4(0, 0, 0), looking_at, Vector4(0, 1, 0), 45, 45,
                  Point4(0, 0, 5));

    Scene scene;

    // scene.addObject(bot_bound);
    // scene.addObject(top_bound);
    // scene.addObject(right_bound);
    // scene.addObject(left_bound);
    // scene.addObject(forward_bound);

    // // scene.addObject(car);
    // scene.addObject(skull);

    // scene.addObject(light_ball);
    scene.addObject(ball1);
    scene.addObject(ball2);
    scene.addObject(ball3);
    /*scene.addObject(triangle1);
    scene.addObject(triangle2);
    scene.addObject(triangle3);
    scene.addObject(light_ball);*/
    scene.addLights(light_ball);
    scene.addLights(light_ball2);
    // scene.addObject(light_ball2);
    scene.setCamera(camera);
    // scene.addLights(light_ball);
    // scene.addLights(light_ball2, false);
    // scene.addLights(light_tri, false);
    // scene.addObject(wood_ball);
    // scene.addObject(sky_ball);
    scene.addObject(table);
    scene.addObject(bot_bound);
    scene.addLights(top_bound);
    scene.addObject(right_bound);
    scene.addObject(left_bound);
    scene.addObject(forward_bound);
    scene.addObject(skull);
    // scene.addObject(a_bound);
    // scene.addObject(b_bound);
    // scene.addObject(c_bound);

    time_t start = std::time(nullptr);
    auto img = computeScene(scene, 1080, 1080);
    time_t end = std::time(nullptr);
    printTimeTaken(start, end, "");

    std::cout << "Saving Image" << std::endl;
    img.save_image("results/" + std::string(argv[1]) + ".ppm");

    return 0;
}
