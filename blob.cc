#include "blob.hh"

#include <cmath>

void Blob::add_potential_point(const Point4& p)
{
    potential_points.push_back(p);
}

void Blob::set_cube_points(Cube& p)
{
    cube = p;
}

void Blob::set_discretisation(float d_)
{
    d = d_;
}

void Blob::set_threshold(float threshold_)
{
    threshold = threshold_;
}

float distance_(const Point4& p, const Point4& q)
{
    auto dx = (q.x - p.x);
    auto dy = (q.y - p.y);
    auto dz = (q.z - p.z);

    return dx * dx + dy * dy + dz * dz;
}

float Blob::compute_potential(Point4& p)
{
    float sum = 0.f;
    for (const auto& pp : potential_points)
    {
        auto d = distance_(pp, p);

        if (d < 1e-6f) return 1e10f; // guard against division by zero
        sum += 1 / std::sqrt(d);
    }

    return sum;
}

Point4 middle(Point4& start, Point4& end)
{
    return Point4((start.x + end.x) / 2, (start.y + end.y) / 2, (start.z + end.z) / 2);
}

Vector4 Blob::compute_gradient(Point4& p)
{
    float eps = 0.01f;
    Point4 px1(p.x + eps, p.y, p.z);
    Point4 px2(p.x - eps, p.y, p.z);
    Point4 py1(p.x, p.y + eps, p.z);
    Point4 py2(p.x, p.y - eps, p.z);
    Point4 pz1(p.x, p.y, p.z + eps);
    Point4 pz2(p.x, p.y, p.z - eps);

    return Vector4(
        compute_potential(px2) - compute_potential(px1),
        compute_potential(py2) - compute_potential(py1),
        compute_potential(pz2) - compute_potential(pz1)
    );
}

Point4 index_to_coord(int& val, Cube& cube)
{
    switch(val)
    {
        case 0:  return middle(cube.points[0], cube.points[1]); // bottom front
        case 1:  return middle(cube.points[1], cube.points[2]); // bottom right
        case 2:  return middle(cube.points[2], cube.points[3]); // bottom back
        case 3:  return middle(cube.points[3], cube.points[0]); // bottom left
        case 4:  return middle(cube.points[4], cube.points[5]); // top front
        case 5:  return middle(cube.points[5], cube.points[6]); // top right
        case 6:  return middle(cube.points[6], cube.points[7]); // top back
        case 7:  return middle(cube.points[7], cube.points[4]); // top left
        case 8:  return middle(cube.points[0], cube.points[4]); // left front vertical
        case 9:  return middle(cube.points[1], cube.points[5]); // right front vertical
        case 10: return middle(cube.points[2], cube.points[6]); // right back vertical
        case 11: return middle(cube.points[3], cube.points[7]); // left back vertical
        default:
            throw std::runtime_error("Invalid value to convert");
    }
}

void Blob::array_to_triangle(std::vector<Triangle>& vec, int config[15], Cube& cube)
{
    int index = 0;
    while (index <= 12 && config[index] != -1)
    {
        auto p1 = index_to_coord(config[index], cube);
        auto p2 = index_to_coord(config[index + 1], cube);
        auto p3 = index_to_coord(config[index + 2], cube);
        
        auto tri = Triangle(p1, p2, p3);

        auto n1 = compute_gradient(p1);
        auto n2 = compute_gradient(p2);
        auto n3 = compute_gradient(p3);

        // Average the three vertex gradients
        tri.normal = Vector4(
            ((n1.x + n2.x + n3.x) / 3.f),
            ((n1.y + n2.y + n3.y) / 3.f),
            ((n1.z + n2.z + n3.z) / 3.f)
        );
        tri.normal.normalize();

        vec.push_back(tri);

        index += 3;
    }
}

std::vector<Triangle> Blob::marching_cubes()
{
    int nb_iter = std::sqrt(distance_(cube.points[0], cube.points[1])) / d;
    std::cout << "nb_iter: " << nb_iter << std::endl;
    std::vector<Triangle> triangles;
    // TODO: For closer approximation
    // should normalize each vector
    // and add it so we can rotate the cube
    for (int x = 0; x < nb_iter; x++)
    {
        for (int y = 0; y < nb_iter; y++)
        {
            for (int z = 0; z < nb_iter; z++)
            {
                auto new_x = cube.points[0].x + x * d;
                auto new_y = cube.points[0].y + y * d;
                auto new_z = cube.points[0].z + z * d;

                // We presuppose it works because nb_iter should allow it
                Cube marching;
                marching.points[0] = Point4(new_x, new_y, new_z);
                marching.points[1] = Point4(new_x + d, new_y, new_z);
                marching.points[2] = Point4(new_x + d, new_y, new_z + d);
                marching.points[3] = Point4(new_x, new_y, new_z + d);
                marching.points[4] = Point4(new_x, new_y + d, new_z);
                marching.points[5] = Point4(new_x + d, new_y + d, new_z);
                marching.points[6] = Point4(new_x + d, new_y + d, new_z + d);
                marching.points[7] = Point4(new_x, new_y + d, new_z + d);

                // Formula given in subject
                int index = 0;
                if (compute_potential(marching.points[0]) > threshold) index |= 1;
                if (compute_potential(marching.points[1]) > threshold) index |= 2;
                if (compute_potential(marching.points[2]) > threshold) index |= 4;
                if (compute_potential(marching.points[3]) > threshold) index |= 8;
                if (compute_potential(marching.points[4]) > threshold) index |= 16;
                if (compute_potential(marching.points[5]) > threshold) index |= 32;
                if (compute_potential(marching.points[6]) > threshold) index |= 64;
                if (compute_potential(marching.points[7]) > threshold) index |= 128;

                array_to_triangle(triangles, configurations[index], marching);
            }
        }
    }

    return triangles;
}
