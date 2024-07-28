#ifndef str_transform_hpp
#define str_transform_hpp

#include "src/include/linalg.hpp"

namespace str
{

class Transform
{
  public:
    Transform() = default;
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
    la::vec<3> size = { 1.0, 1.0, 1.0 };
    la::vec<3> position = { 0.0, 0.0, 0.0 };
    la::vec<3> rotation = { 0.0, 0.0, 0.0 };
};

} // namespace str

#endif // str_transform_hpp