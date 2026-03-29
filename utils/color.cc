#include "color.hh"

Color::Color(const Canal& canal_, const uint8_t& value_)
    : canal(canal_), value(value_)
{};

Color::Color(const Color& c)
{
    this->canal = c.canal;
    this->value = c.value;
}

Pixel::Pixel()
{
    colors.reserve(3);
    colors[0] = 0;
    colors[1] = 0;
    colors[2] = 0;
}

Pixel::Pixel(const Pixel& p)
{
    this->colors.reserve(3);
    
    this->colors[0] = p.colors[0];
    this->colors[1] = p.colors[1];
    this->colors[2] = p.colors[2];
}

Pixel::Pixel(const uint8_t& r, const uint8_t& g, const uint8_t& b)
{
    colors.reserve(3);
    colors[0] = r;
    colors[1] = g;
    colors[2] = b;
}