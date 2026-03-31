#include "color.hh"

Value::Value(const Canal& canal_, const uint8_t& value_)
    : canal(canal_), value(value_)
{};

Value::Value(const Value& c)
{
    this->canal = c.canal;
    this->value = c.value;
}

Color::Color()
    : colors(3, 0)
{}

Color::Color(const uint8_t& r, const uint8_t& g, const uint8_t& b)
    : colors{r, g, b}
{}

Color::Color(const Color& p)
    : colors(p.colors)
{}
