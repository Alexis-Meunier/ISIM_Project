#pragma once

#include <vector>

#include "../utils/color.hh"

class Image
{
public:
    int height;
    int width;
    std::vector<Pixel*> pixels;

protected:
    Image() = default;
};

class PPM : public Image
{
public:
    PPM() = default;
    PPM(const int& height, const int& width);

    static PPM load_image(const std::string& filename);
    void save_image(const std::string& filename);

    int maxval;

};