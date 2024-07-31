#ifndef str_camera_hpp
#define str_camera_hpp

#include "src/include/linalg.hpp"

#include <vecs/vecs.hpp>

namespace str
{

class Camera
{
  friend class Renderer;

  public:
    Camera();
    Camera(const Camera&) = default;
    Camera(Camera&&) = default;

    ~Camera() = default;

    Camera& operator = (const Camera&) = default;
    Camera& operator = (Camera&&) = default;

    const la::mat<4> model() const;
    const la::mat<4> view() const;
    const la::mat<4>& projection() const;
    const la::vec<3> params() const;

    void translate(la::vec<3>);
    void rotate(la::vec<3>);

  private:
    la::mat<4> mat_projection = la::mat<4>::identity();

    la::vec<3> position = { 0.0, 0.0, 1.0 };
    la::vec<3> rotation = { 0.0, 0.0, 0.0 };
};

} // namespace str

#endif //str_camera_hpp