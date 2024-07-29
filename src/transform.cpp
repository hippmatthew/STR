#include "src/include/transform.hpp"

namespace str
{

Transform::Transform(Shape sh, float sm, la::vec<3> col)
{
  shape = sh;

  if (sm < 0) sm = 0;
  else if (sm > 1) sm = 1;

  smoothess = sm;
  color = col;
}

const la::mat<4> Transform::model() const
{
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