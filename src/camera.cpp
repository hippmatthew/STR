#include "src/include/camera.hpp"

namespace str
{

Camera::Camera()
{
  float aspect_ratio = VECS_SETTINGS.width() / VECS_SETTINGS.height();

  mat_view = la::mat<4>::view_matrix({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
  mat_projection = la::mat<4>::perspective_projection(la::radians(60.0f), aspect_ratio, 0.1f, 10.0f);
}

const la::mat<4>& Camera::view() const
{
  return mat_view;
}

const la::mat<4>& Camera::projection() const
{
  return mat_projection;
}

} // namespace str