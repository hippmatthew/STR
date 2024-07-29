#ifndef str_transform_hpp
#define str_transform_hpp

#include "src/include/linalg.hpp"

namespace str
{

enum Shape
{
  Sphere = 0u,
  Cube
};

class Transform
{
  public:
    Transform(Shape sh = Shape::Sphere, float sm = 0.5f, la::vec<3> col = { 0.0, 1.0, 0.0 });
    Transform(const Transform&) = default;
    Transform(Transform&&) = default;

    ~Transform() = default;

    Transform& operator = (const Transform&) = default;
    Transform& operator = (Transform&&) = default;

    const la::mat<4> model() const;

    Transform& scale(la::vec<3>);
    Transform& translate(float, la::vec<3>);
    Transform& rotate(la::vec<3>);

  private:
    alignas(4) unsigned int shape = Shape::Sphere;
    alignas(4) float smoothess = 0.5;
    alignas(16) la::vec<3> color = { 0.0, 1.0, 0.0 };
    alignas(16) la::vec<3> size = { 1.0, 1.0, 1.0 };
    alignas(16) la::vec<3> position = { 0.0, 0.0, 0.0 };
    alignas(16) la::vec<3> rotation = { 0.0, 0.0, 0.0 };
};

} // namespace str

#endif // str_transform_hpp