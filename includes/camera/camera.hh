#pragma once

#include "utils/vector4.hh"
#include "utils/point4.hh"

class Camera
{
public:
    Camera() = default;
    Camera(const Point4& center_, const Vector4& looking_at,
           const Vector4& up, const double& open_angle_x,
           const double& open_angle_y, const Point4& zmin);
    Camera(const Camera& cam);

    Camera& operator=(const Camera& cam);

    Point4 center;
    Vector4 looking_at;
    Vector4 up;
    double open_angle_x;
    double open_angle_y;
    Point4 zmin;
};


std::pair<Vector4, Vector4> get_basis(const Camera& cam);

std::pair<float, float> get_camera_plane(const Camera& cam);