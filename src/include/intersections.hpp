#ifndef str_intersection_hpp
#define str_intersection_hpp

#include "src/include/linalg.hpp"

namespace str
{

struct Ray
{
  la::vec<3> origin = { 0.0, 0.0, 0.0 };
  la::vec<3> direction = { 0.0, 0.0, 1.0 };
  la::vec<3> color = { 0.0, 0.0, 0.0 };
};

class Intersection
{
  public:
    struct Info
    {
      bool hit;
      la::vec<3> point = { 0.0, 0.0, 0.0 };
      la::vec<3> normal = { 0.0, 0.0, 0.0 };
      la::vec<3> color = { 0.0, 0.0, 0.0 };
    };

  public:
    Intersection() = default;
    Intersection(const Intersection&) = default;
    Intersection(Intersection&&) = default;

    virtual ~Intersection() = default;

    Intersection& operator = (const Intersection&) = default;
    Intersection& operator = (Intersection&&) = default;

    virtual Intersection::Info operator () (Ray) = 0;
};

class Sphere : public Intersection
{
  public:
    Sphere(la::vec<3>, float, la::vec<3>);

    Intersection::Info operator () (Ray);

  private:
    const la::vec<3> center;
    const float radius;
    const la::vec<3> color;
};

class Cube : public Intersection
{
  public:
    Cube(la::vec<3>, float, la::vec<3>);

    Intersection::Info operator () (Ray);

  private:
    la::vec<3> face(float, la::vec<3>, la::vec<3>) const;

  private:
    const la::vec<3> center;
    const float length;
    const la::vec<3> color;
};

} // namespace str

#endif // str_intersection_hpp