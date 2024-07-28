#include "src/include/transform.hpp"

namespace str
{

const la::mat<4> Transform::model() const
{
  // la::mat<4> Rx{
  //   { 1.0, 0.0, 0.0, 0.0 },
  //   { 0.0, cos(rotation[0]), sin(rotation[0]), 0.0 },
  //   { 0.0, -sin(rotation[0]), cos(rotation[0]), 0.0 },
  //   { 0.0, 0.0, 0.0, 1.0 }
  // };

  // la::mat<4> Ry{
  //   { cos(rotation[1]), 0.0, -sin(rotation[1]), 0.0 },
  //   { 0.0, 1.0, 0.0, 0.0 },
  //   { sin(rotation[1]), 0.0, cos(rotation[1]), 0.0 },
  //   { 0.0, 0.0, 0.0, 1.0 }
  // };

  // la::mat<4> Rz{
  //   { cos(rotation[2]), sin(rotation[2]), 0.0, 0.0 },
  //   { -sin(rotation[2]), cos(rotation[2]), 0.0, 0.0 },
  //   { 0.0, 0.0, 1.0, 0.0 },
  //   { 0.0, 0.0, 0.0, 1.0 }
  // };

  la::mat<4> Rx = la::mat<4>::rotation_matrix(rotation[0], { 1.0, 0.0, 0.0 });
  la::mat<4> Ry = la::mat<4>::rotation_matrix(rotation[1], { 0.0, -1.0, 0.0 });
  la::mat<4> Rz = la::mat<4>::rotation_matrix(rotation[2], { 0.0, 0.0, 1.0 });

  la::mat<4> T = la::mat<4>::translation_matrix(position);
  la::mat<4> R = Rz * Ry * Rx;
  la::mat<4> S = la::mat<4>::scale_matrix(size[0], size[1], size[2]);

  return T * R * S;
}

Transform& Transform::scale(la::vec<3> s)
{
  size = size + s;
  return *this;
}

Transform& Transform::translate(float mag, la::vec<3> dir)
{
  position = position + mag * dir.normalized();
  return *this;
}

Transform& Transform::rotate(la::vec<3> r)
{
  rotation = rotation + r;
  return *this;
}

} // namespace str