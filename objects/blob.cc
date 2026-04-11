#include "blob.hh"

void Blob::add_vertices(std::vector<Point4> vertices)
{
    for (Point4 vertex : vertices)
    {
        this->vertices.push_back(vertex);
    }
}

float Blob::potentiel(const Point4& point)
{
    float distances = 0;
    for (const Point4& vertex : vertices)
    {
        distances += point.distance(vertex);
    }
    distances /= vertices.size();
    return 1 / (distances * distances);
}

std::vector<Triangle> Blob::marching_cubes()
{
    std::vector<Triangle> tr = std::vector<Triangle>();
    const auto uue = Point4{ 0, e, 0 }; // unity up e
    const auto ure = Point4{ e, 0, 0 }; // unity right e
    const auto ufe = Point4{ 0, 0, e }; // unity forward e
    for (float y = 0; y < e; y += d)
    {
        for (float z = 0; z < e; z += d)
        {
            for (float x = 0; x < e; x += d)
            {
                const Point4 p7 =
                    bottom_backward_left + uue * y + ufe * z + ure * x;
                const Point4 p4 = p7 + ufe;
                const Point4 p5 = p4 + ure;
                const Point4 p6 = p7 + ure;
                const Point4 p3 = p7 + uue;
                const Point4 p0 = p3 + ufe;
                const Point4 p1 = p0 + ure;
                const Point4 p2 = p3 + ure;
                int index = 0;
                if (potentiel(p0) < threshold)
                    index |= 1;
                if (potentiel(p1) < threshold)
                    index |= 2;
                if (potentiel(p2) < threshold)
                    index |= 4;
                if (potentiel(p3) < threshold)
                    index |= 8;
                if (potentiel(p4) < threshold)
                    index |= 16;
                if (potentiel(p5) < threshold)
                    index |= 32;
                if (potentiel(p6) < threshold)
                    index |= 64;
                if (potentiel(p7) < threshold)
                    index |= 128;
                // TODO
            }
        }
    }

    return tr;
}
