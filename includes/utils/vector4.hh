#pragma once

#include <iostream>

#include "point4.hh"

class Vector4
{
public:
    Vector4();
    Vector4(const float& x_, const float& y_, const float& z_, const float& i_);
    Vector4(const Vector4& v);
    Vector4(const Point4& p);
    Vector4(const float& x_, const float& y_, const float& z_);

    // Arithmetic
    Vector4 operator+(const Vector4& p) const;
    Vector4 operator-(const Vector4& p) const;
    Vector4 operator*(const double& scalar) const; // Translation
    float operator*(const Vector4& p) const; // Dot product

    Vector4 operator+(const Point4& p) const;
    Vector4 operator-();
    Vector4& operator=(const Vector4& v);
    
    // Debugging
    std::ostream& operator<<(std::ostream& os);

    // Indexing
    float& operator[](size_t idx);
    float operator[](size_t idx) const;

    void normalize();

    float x;
    float y;
    float z;
    float i;
};

std::ostream& operator<<(std::ostream& out, Vector4& vect);

/**
 * Returns the result of the cross product between v1 and v2
 *
 * Reminder:
 *      In R^3:
 *          v1 = (x1, y1, z1)
 *          v2 = (x2, y2, z2)
 *
 * ==>  | i  j  k  |
 *      | x1 y1 z1 |
 *      | x2 y2 z2 |
 *
 * v1 x v2 =
 *      i * (y1 * z2 - z1 * y2),
 *    - j * (x1 * z2 - x2 * z1),
 *    + k * (x1 * y2 - x2 * y1)
 */
Vector4 cross_product(Vector4& v1, const Vector4& v2);

/**
 * Computes the dot product between two vector
 *
 * Reminder:
 *      In R^3:
 *          v1 = (x1, y1, z1)
 *          v2 = (x2, y2, z2)
 *
 * <v1, v2> = x1*x2 + y1*y2 + z1*z2
 *
 * N.B.: Here the function calls the overrident * operator
 *       and clamps it to zero
 */
float dot_product(Vector4& light, const Vector4& point);

#include "vector4.hxx"
