#include "camera.hh"

Camera::Camera(const Point4& center_, const Vector4& looking_at_,
           const Vector4& up_, const double& open_angle_x_,
           const double& open_angle_y_, const Point4& zmin_)
    : center(center_), looking_at(looking_at_), up(up_),
      open_angle_x(open_angle_x_), open_angle_y(open_angle_y_),
      zmin(zmin_)
{}

Camera::Camera(const Camera& cam)
{
    this->center = cam.center;
    this->looking_at = cam.looking_at;
    this->up = cam.up;
    this->open_angle_x = cam.open_angle_x;
    this-> open_angle_y = cam.open_angle_y;
    this->zmin = cam.zmin;
}

