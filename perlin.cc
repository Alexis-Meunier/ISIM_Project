#include "perlin.hh"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

// Compute Random gradient at grid corners
std::pair<float, float> gradient(float h)
{
    return { std::cos(h), std::sin(h) };
}

// Linear interpolation
float lerp(float a, float b, float x)
{
    return a + x * (b - a);
}

// smootherStep
float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float perlin(float x, float y, std::vector<std::pair<float, float>>& gradients, int grid_size, int nb_col, int nb_row)
{
    auto x0 = int(x / grid_size);
    auto y0 = int(y / grid_size);

    auto x1 = x0 + 1;
    auto y1 = y0 + 1;

    x1 = std::min(x1, nb_col - 1);
    y1 = std::min(y1, nb_row - 1);

    auto dx = x / grid_size - x0;
    auto dy = y / grid_size - y0;

    auto dot00 = gradients[y0 * nb_col + x0].first * dx + gradients[y0 * nb_col + x0].second * dy;
    auto dot10 = gradients[y0 * nb_col + x1].first * (dx-1) + gradients[y0 * nb_col + x1].second * dy;
    auto dot01 = gradients[y1 * nb_col + x0].first * dx + gradients[y1 * nb_col + x0].second * (dy-1);
    auto dot11 = gradients[y1 * nb_col + x1].first * (dx-1) + gradients[y1 * nb_col + x1].second * (dy-1);

    auto u = fade(dx);
    auto v = fade(dy);

    return lerp(lerp(dot00, dot10, u), lerp(dot01, dot11, u), v);
}

std::vector<std::pair<float, float>> load_gradients(int nb_col, int nb_row)
{
    std::vector<std::pair<float, float>> gradients;
    gradients.resize(nb_col * nb_row);

    for (int y = 0; y < nb_row; y++)
    {
        for (int x = 0; x < nb_col; x++)
        {
            gradients[y * nb_col + x] = gradient(static_cast<float>(std::rand()) / RAND_MAX * 2 * M_PI);
        }
    }

    return gradients;
}

PPM createRandomImage(int sx, int sy, int grid_size)
{
    PPM img(sx, sy);

    int nb_col = std::ceil((float)img.width / grid_size) + 1;
    int nb_row = std::ceil((float)img.height / grid_size) + 1;

    auto gradients = load_gradients(nb_col, nb_row);

    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            // Return value [-1, 1]
            float n = perlin(x, y, gradients, grid_size, nb_col, nb_row);

            // Clamp it back to [0, 1]
            float normalized = (n + 1.f) * 0.5f;

            // Multiply by 255 to get the grey value
            int gray = (int)(normalized * 255.f);

            img.pixels[y * img.width + x] = Color(gray, gray, gray);
        }
    }

    return img;
}

void procedural(int sx = 100, int sy = 100, int nb_images = 5, float weight = 0.5f, int grid_size = 8)
{
    std::vector<float> buffer(sx * sy, 0.0f);

    float currentWeight = 1.0f;

    for (int i = 0; i < nb_images; i++)
    {
        auto im = createRandomImage(sx, sy, grid_size);

        for (int y = 0; y < sy; y++)
        {
            for (int x = 0; x < sx; x++)
            {
                float col = 255.f - im.pixels[y * sx + x].colors[RED];

                col /= 255.f;

                buffer[y * sx + x] += currentWeight * col;
            }
        }

        currentWeight *= weight;
    }

    float minVal = 1e9f;
    float maxVal = -1e9f;

    for (float v : buffer)
    {
        minVal = std::min(minVal, v);
        maxVal = std::max(maxVal, v);
    }

    PPM img(sx, sy);

    for (int i = 0; i < sx * sy; i++)
    {
        float norm = (buffer[i] - minVal) / (maxVal - minVal);

        norm = std::pow(norm, 1.3f);

        int gray = (int)(norm * 255.f);

        img.pixels[i] = Color(gray, gray, gray);
    }

    img.save_image("results/procedural.ppm");
}
