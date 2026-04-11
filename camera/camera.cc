#include "camera.hh"

#include <cmath>

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

std::pair<Vector4, Vector4> get_basis(const Camera& cam)
{
    auto straight = cam.looking_at;
    auto up = cam.up;

    if (straight * up > 0.999)
        up = Vector4(0, 0, 1);

    // S = U.V
    auto horizontal = cross_product(straight, up);
    horizontal.normalize();

    // U' = S.U
    auto real_up = cross_product(horizontal, straight);
    real_up.normalize();

    return { horizontal, real_up };
}

std::pair<float, float> get_camera_plane(const Camera& cam)
{
    float dist_to_zmin = distance(cam.zmin, cam.center);

    // Physics: I drew it on a board to be sure
    float W = std::tan(cam.open_angle_x) * dist_to_zmin;
    float H = std::tan(cam.open_angle_y) * dist_to_zmin;

    return { W, H };
}

Camera& Camera::operator=(const Camera& cam)
{
   center = cam.center;
   looking_at = cam.looking_at;
   up = cam.up;
   open_angle_x = cam.open_angle_x;
   open_angle_y = cam.open_angle_y;
   zmin = cam.zmin;

   return *this;
}
