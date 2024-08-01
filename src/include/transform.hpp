#ifndef str_transform_hpp
#define str_transform_hpp

#include "src/include/linalg.hpp"

namespace str
{

struct Material
{
  la::vec<3> color;
};

class Transform
{
  public:
    Transform(la::vec<3> c = { 0.0, 1.0, 0.0 });
    Transform(const Transform&) = default;
    Transform(Transform&&) = default;

    ~Transform() = default;

    Transform& operator = (const Transform&) = default;
    Transform& operator = (Transform&&) = default;

    const la::mat<4> model() const;

    Transform& scale(la::vec<3>);
    Transform& translate(float, la::vec<3>);
    Transform& rotate(la::vec<3>);

    const la::vec<3>& pos() const { return position; }

  private:
    alignas(16) la::vec<3> position = { 0.0, 0.0, 0.0 };
    alignas(16) la::vec<3> rotation = { 0.0, 0.0, 0.0 };
    alignas(16) la::vec<3> size = { 1.0, 1.0, 1.0 };
    alignas(16) la::vec<3> color = { 0.0, 1.0, 0.0 };
};

} // namespace str

#endif // str_transform_hpp