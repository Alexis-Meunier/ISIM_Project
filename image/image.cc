#include "image.hh"

#include <fstream>

PPM::PPM(const int& height, const int& width)
{
    this->maxval = 255;
    this->height = height;
    this->width = width;
    this->pixels.reserve(height * width);
}


PPM PPM::load_image(const std::string& filename)
{
    std::ifstream is(filename, std::ios::binary);
    if (!is.is_open())
    {
        std::cerr << "Could not load the image '" << filename << "'\n";
        return PPM();
    }

    PPM img;
    std::string val;

    is >> val;
    is >> img.width;
    is >> img.height;
    is >> img.maxval;

    img.pixels.resize(img.width * img.height);
    char *width = new char[img.width * img.height * 3];

    is.get();
    is.read(width, img.width * img.height * 3);

    std::cout << img.width << "x" << img.height << std::endl;

    for (auto i = 0; i < img.height; i++)
    {
        for (auto j = 0; j < img.width; j++)
        {
            auto index = (i * img.width + j) * 3;

            uint8_t r = width[index];
            std::cout << "loading r: " << int(r) << std::endl;

            uint8_t g = width[index + 1];
            std::cout << "loading g: " << int(g) << std::endl;

            uint8_t b = width[index + 2];
            std::cout << "loading b: " << int(b) << std::endl;

            img.pixels[i * img.width + j] = Color(r, g, b);
            std::cout << "loading pixel: " << i * img.width + j << std::endl;
        }
        std::cout << "finished row" << std::endl << std::endl;
    }

    return img;
}


void PPM::save_image(const std::string& filename)
{
    std::ofstream os(filename, std::ios::binary);
    if (!os.is_open())
    {
        std::cerr << "Could not save the image '" << filename << "'\n";
        return;
    }

    os << "P6\n" << width << " " << height << "\n" << maxval << "\n";
    for (auto i = 0; i < height; i++)
    {
        for (auto j = 0; j < width; j++)
        {
            Color p = this->pixels[i * width + j];
            os << p.colors[0] << p.colors[1] << p.colors[2];
        }
    }
}
