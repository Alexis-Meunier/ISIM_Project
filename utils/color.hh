#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

enum Canal
{
    RED = 0,
    GREEN,
    BLUE,
    GRAY
};

class Value
{
public:
    Value()
        : canal(GRAY)
        , value(0) {};
    Value(const Canal& canal_, const uint8_t& value_);
    Value(const Value& c);

    Canal canal;
    uint8_t value;
};

class Color
{
public:
    Color();
    Color(const uint8_t& r, const uint8_t& g, const uint8_t& b);
    Color(const Color& p);

    uint8_t operator[](const int& idx) const;
    uint8_t& operator[](const int& idx);
    Color operator+(const Value& c);
    Color operator-(const Value& c);
    Color operator*();
    std::ostream& operator<<(std::ostream& os);

    std::vector<uint8_t> colors;
};

std::ostream& operator<<(std::ostream& out, Color& vect);

#include "color.hxx"
