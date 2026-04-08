#pragma once

#include <iostream>

class Point4
{
    public:
        Point4();
        Point4(const float& x_, const float& y_, const float& z_, const float& i_);
        Point4(const Point4& v);
        Point4(const float& x_, const float& y_, const float& z_);

        void rotateX(const float& angle);
        void rotateY(const float& angle);
        void rotateZ(const float& angle);

        // Arithmetic
        Point4 operator+(const Point4& p);
        Point4 operator-(const Point4& p);
        Point4 operator*(const double& scalar); // Translation
        float operator*(const Point4& p); // Dot product
        Point4& operator=(const Point4& v);

        // Debugging
        std::ostream& operator<<(std::ostream& os);

        // Indexing
        float& operator[](size_t idx);
        float operator[](size_t idx) const;

        float x;
        float y;
        float z;
        float i;
};

/**
 * Returns the euclidian norm/Distance between two points
 * 
 * Reminder:                    
 *      In R^3:
 *          v1 = (x1, y1, z1)
 *          v2 = (x2, y2, z2) 
 * 
 * ||v1 - v2|| = sqrt(
 *      (x1 - x2)^2
 *    + (y1 - y2)^2
 *    + (z1 - z2)^2
 * )
 */
float distance(const Point4& p1, const Point4& p2);

#include "point4.hxx"
