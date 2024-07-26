#ifndef str_camera_hpp
#define str_camera_hpp

#include "src/include/linalg.hpp"

#include <vecs/vecs.hpp>

namespace str
{

class Camera
{
  public:
    Camera();
    Camera(const Camera&) = default;
    Camera(Camera&&) = default;

    ~Camera() = default;

    Camera& operator = (const Camera&) = default;
    Camera& operator = (Camera&&) = default;

    const la::mat<4>& view() const;
    const la::mat<4>& projection() const;

  private:
    la::mat<4> mat_view = la::mat<4>::identity();
    la::mat<4> mat_projection = la::mat<4>::identity();
};

} // namespace str

#endif //str_camera_hpp