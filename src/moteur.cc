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
        if (sh && so != light_obj && !so->get_texture(*sh)->kr
            && so->get_texture(*sh)->lightPower == 0)
        {
            float to_blocker = distance(offset_hit, *sh);
            if (to_blocker < dist_to_light - 0.01f)
                continue; // in shadow
        }

        float power = li->lightPower * cos_theta * info->kd;
        direct_r += (li->color->colors[RED] / 255.f) * power * ar;
        direct_g += (li->color->colors[GREEN] / 255.f) * power * ag;
        direct_b += (li->color->colors[BLUE] / 255.f) * power * ab;
    }

    if (direct_b > 1 || direct_g > 1 || direct_r > 1)
    {
        direct_r = std::min(1.0f, direct_r);
        direct_g = std::min(1.0f, direct_g);
        direct_b = std::min(1.0f, direct_b);
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

    float rd = (static_cast<float>(rand()) / RAND_MAX);
    float cos_dir_norm = std::min(std::abs(ray * normal), 1.0f);
    float kr = 1;
    if (info->kr)
    {
        kr = fresnel(cos_dir_norm, etai, etat);
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

    if (indirect_b > 1 || indirect_g > 1 || indirect_r > 1)
    {
        indirect_r = std::min(1.0f, indirect_r);
        indirect_g = std::min(1.0f, indirect_g);
        indirect_b = std::min(1.0f, indirect_b);
    }

    return Color(indirect_r * 255, indirect_g * 255, indirect_b * 255);
}

Color compute_light_rays(std::shared_ptr<TextureMaterial>& light_tex,
                         const Point4& hit, int depth)
{
    auto li = light_tex->get_elements(hit);
    float mini = std::min(1.0f, li->lightPower);
    return Color(li->color->colors[RED] * mini, li->color->colors[GREEN] * mini,
                 li->color->colors[BLUE] * mini);
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

    float contribution_r = direct_r + indirect_r;
    float contribution_g = direct_g + indirect_g;
    float contribution_b = direct_b + indirect_b;

    if (contribution_b > 1 || contribution_g > 1 || contribution_r > 1)
    {
        contribution_r = std::min(1.0f, contribution_r);
        contribution_g = std::min(1.0f, contribution_g);
        contribution_b = std::min(1.0f, contribution_b);
    }

    return Color(std::min(255.f, contribution_r * 255.f),
                 std::min(255.f, contribution_g * 255.f),
                 std::min(255.f, contribution_b * 255.f));
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
    light.kd = 1.0f;
    light.color = new Color(255, 255, 255);
    light.lightPower = 1.0f;
    TextureInfo light2;
    light2.kd = 1.0f;
    light2.color = new Color(200, 255, 0);
    light2.lightPower = 0.5f;
    TextureInfo stronk_light;
    stronk_light.kd = 0.5f;
    stronk_light.ks = 0.2f;
    stronk_light.color = new Color(255, 255, 255);
    stronk_light.lightPower = 0.9f;

    time_t load_textureInfo = std::time(nullptr);
    printTimeTaken(load_start, load_textureInfo,
                   "while loading texture Infos.");

    // ── Textures (unchanged) ──────────────────────────────────────────────────
    auto new_text = std::make_shared<UniformTexture>(Color{ 255, 255, 255 }, 0,
                                                     1, true, 1.5); // glass
    auto new_textr = std::make_shared<UniformTexture>(Color{ 255, 255, 255 }, 0,
                                                      1, false, 1.5); // mirror
    auto sky_light = std::make_shared<ProceduralTexture>(ProceduralType::CLOUD,
                                                         1, 0, 0.8, true);
    auto wood = std::make_shared<ProceduralTexture>(ProceduralType::WOOD, 1, 0);
    auto image_texture = std::make_shared<ImageTexture>("textures/image.jpg");
    auto uniform_flat_blue =
        std::make_shared<UniformTexture>(Color(0, 100, 255));
    auto uniform_flat_green =
        std::make_shared<UniformTexture>(Color(100, 255, 37));
    auto uniform_flat_cyan =
        std::make_shared<UniformTexture>(Color(0, 255, 255));
    auto uniform_metal_cyan =
        std::make_shared<UniformTexture>(Color(100, 255, 255), 0.5, 0.5);
    auto uniform_mat_red = std::make_shared<UniformTexture>(&mat_red);
    auto light_texture = std::make_shared<LightTexture>(&light);
    auto light_texture2 = std::make_shared<LightTexture>(&light2);

    // ── Room bounds (Cornell Box, same as before) ────────────────────────────
    auto bot_bound =
        Mesh::rectangle(Point4(-40, -20, 50), Point4(40, -20, 50),
                        Point4(40, -20, 0), Point4(-40, -20, 0), new_textr);
    auto left_bound = Mesh::rectangle(
        Point4(-40, 20, -100), Point4(-40, 20, 50), Point4(-40, -20, 50),
        Point4(-40, -20, -100), uniform_flat_cyan);
    auto right_bound = Mesh::rectangle(Point4(40, 20, 50), Point4(40, 20, -100),
                                       Point4(40, -20, -100),
                                       Point4(40, -20, 50), uniform_metal_cyan);
    auto top_bound =
        Mesh::rectangle(Point4(-40, 20, -100), Point4(40, 20, -100),
                        Point4(40, 20, 50), Point4(-40, 20, 50), sky_light);
    auto forward_bound = Mesh::rectangle(
        Point4(-100, 20, 35), Point4(100, 20, 35), Point4(100, -20, 35),
        Point4(-100, -20, 35), image_texture);

    // ── Ball 1: glass sphere, LEFT side ──────────────────────────────────────
    const int r1 = 6;
    const int x1 = -15, y1 = -20 + r1, z1 = 22;
    Sphere ball1(new_text, Point4(x1, y1, z1), r1); // glass

    // Pedestal/table under ball1 — height 5, same footprint as ball radius
    auto wood_table =
        Mesh::cube(Point4(x1 - r1, -17, z1 + r1), Point4(x1 + r1, -17, z1 + r1),
                   Point4(x1 + r1, -17, z1 - r1), Point4(x1 - r1, -17, z1 - r1),
                   3, // thickness (height)
                   wood);

    // ── Ball 2: mirror sphere, RIGHT side ────────────────────────────────────
    const int r2 = 5;
    const int x2 = 15, y2 = -20 + r2, z2 = 22;
    Sphere reflect_ball(wood, Point4(x2, y2, z2), r2); // mirror

    auto glass_table =
        Mesh::cube(Point4(x2 - r2, -17, z2 + r2), Point4(x2 + r2, -17, z2 + r2),
                   Point4(x2 + r2, -17, z2 - r2), Point4(x2 - r2, -17, z2 - r2),
                   3, new_textr);

    // ── Small decorative red ball in the centre-back ─────────────────────────
    Sphere ball_center(uniform_mat_red, Point4(0, -20 + 3, 28), 3);

    // ── Light ball (area light via ceiling, + small fill light) ──────────────
    Sphere light_ball1(light_texture, Point4(-20, 10, 19), 4);
    // Sphere light_ball2(light_texture2, Point4(20, 10, 15), 2);

    // ── Camera ───────────────────────────────────────────────────────────────
    Vector4 looking_at(0, 0, 1);
    looking_at.normalize();
    Camera camera(Point4(0, 0, 0), looking_at, Vector4(0, 1, 0), 45, 45,
                  Point4(0, 0, 5));

    // ── Scene assembly ───────────────────────────────────────────────────────
    Scene scene;

    // Geometry
    scene.addObject(ball1);
    scene.addObject(reflect_ball);
    scene.addObject(ball_center);
    scene.addObject(wood_table);
    scene.addObject(glass_table);

    // Room
    scene.addObject(bot_bound);
    scene.addObject(left_bound);
    scene.addObject(right_bound);
    scene.addObject(forward_bound);

    // Lights (top_bound is an emissive ceiling + one fill light)
    scene.addLights(top_bound);
    scene.addLights(light_ball1);

    scene.setCamera(camera);

    time_t start = std::time(nullptr);
    auto img = computeScene(scene, 800, 1000);
    time_t end = std::time(nullptr);
    printTimeTaken(start, end, "");

    std::cout << "Saving Image" << std::endl;
    img.save_image("results/" + std::string(argv[1]) + ".ppm");

    return 0;
}
