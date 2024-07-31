#include "src/include/camera.hpp"

namespace str
{

Camera::Camera()
{
  mat_projection = la::mat<4>::perspective_projection(
    la::radians(60.0f),
    VECS_SETTINGS.aspect_ratio(),
    0.1f,
    10.0f
  );
}

const la::mat<4> Camera::model() const
{
  auto Rx = la::mat<4>::rotation_matrix(rotation[0], { 1.0, 0.0, 0.0 });
  auto Ry = la::mat<4>::rotation_matrix(rotation[1], { 0.0, -1.0, 0.0 });
  auto Rz = la::mat<4>::rotation_matrix(rotation[2], { 0.0, 0.0, 1.0 });

  return Rz * Ry * Rx * la::mat<4>::translation_matrix(position);
}

const la::mat<4> Camera::view() const
{
  auto Rx = la::mat<4>::rotation_matrix(rotation[0], { 1.0, 0.0, 0.0 });
  auto Ry = la::mat<4>::rotation_matrix(rotation[1], { 0.0, -1.0, 0.0 });
  auto Rz = la::mat<4>::rotation_matrix(rotation[2], { 0.0, 0.0, 1.0 });

  auto normal = Rz * Ry * Rx * la::vec<4>{ 0.0, 0.0, -1.0, 1.0 };

  return la::mat<4>::view_matrix(
    position + la::vec<3>{ normal[0], normal[1], normal[2] }.normalized(),
    position,
    { 0.0, -1.0, 0.0 }
  );
}

const la::mat<4>& Camera::projection() const
{
  return mat_projection;
}

const la::vec<3> Camera::params() const
{
  float nearPlane = 0.1f;
  float planeHeight = 2 * nearPlane * tanf(la::radians(60.0f) / 2.0);
  float planeWidth = VECS_SETTINGS.aspect_ratio() * planeHeight;

  return la::vec<3>{ planeWidth, planeHeight, nearPlane };
}

void Camera::translate(la::vec<3> displacement)
{
  position = position + displacement;
}

void Camera::rotate(la::vec<3> angles)
{
  rotation = rotation + angles;
}

} // namespace str