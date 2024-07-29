#include "src/include/intersections.hpp"
#include <stdexcept>

namespace str
{

Sphere::Sphere(la::vec<3> c, float r, la::vec<3> col) : center(c), radius(r), color(col)
{
  if (r <= 0) throw std::runtime_error("sphere radius less than or equal to 0");
}

Intersection::Info Sphere::operator () (Ray ray)
{
  la::vec<3> L = ray.origin - center;
  la::vec<3> D = ray.direction.normalized();

  float b = 2 * (L * D);
  float c = L * L - radius * radius;
  float discriminant = b * b - 4 * c;

  if (discriminant < 0) return Intersection::Info{ false };

  float sqrt_discriminant = sqrt(b * b / 4 - c);
  float t = fmin(-b / 2 - sqrt_discriminant, -b / 2 + sqrt_discriminant);

  la::vec<3> P = ray.origin + t * D;
  return Intersection::Info{ true, P, (P - center).normalized(), ray.color + color };
}

Cube::Cube(la::vec<3> c, float l, la::vec<3> col) : center(c), length(l), color(col)
{
  if (l <= 0) throw std::runtime_error("cube side length less than or equal to 0");
}

Intersection::Info Cube::operator () (Ray ray)
{
  la::vec<3> r_min = center + length * la::vec<3>{ -1, 1, -1 }.normalized();
  la::vec<3> r_max = center + length * la::vec<3>{ 1, -1, 1 }.normalized();

  la::mat<3> D = la::mat<3>::identity();
  for (unsigned long i = 0; i < 3; ++i)
    if (ray.direction[i] != 0) D[i][i] = 1 / ray.direction[i];

  auto t_min = D * (r_min - ray.origin);
  auto t_max = D * (r_max - ray.origin);

  for (unsigned long i = 0; i < 3; ++i)
    if (t_min[i] > t_max[i]) std::swap(t_min[i], t_max[i]);

  float t_enter = fmax(fmax(t_min[0], t_min[1]), t_min[2]);
  float t_exit = fmin(fmax(t_max[0], t_max[1]), t_max[2]);

  if (!(t_enter <= t_exit && t_exit >= 0)) return Intersection::Info{ false };

  la::vec<3> dir = ray.direction.normalized();
  return Intersection::Info{ true, ray.origin + t_enter * dir, face(t_enter, t_min, dir), ray.color + color };
}

la::vec<3> Cube::face(float t, la::vec<3> t_vec, la::vec<3> D) const
{
  for (unsigned long i = 0; i < 3; ++i)
  {
    if (t != t_vec[i]) continue;

    return D[i] < 0 ? la::vec<3>{ -1.0, 0.0, 0.0 } : la::vec<3>{ 1.0, 0.0, 0.0 };
  }

  return la::vec<3>{ 0.0, 0.0, 0.0 };
}

} // namespace str