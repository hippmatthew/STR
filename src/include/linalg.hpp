#ifndef str_linalg_hpp
#define str_linalg_hpp

#include "src/include/linalg_decl.hpp"

namespace la
{

template <unsigned long N, typename T>
vec<N, T>::vec()
{
  data = zero().data;
}

template <unsigned long N, typename T>
vec<N, T>::vec(std::initializer_list<T> list)
{
  if (list.size() != N)
    throw std::out_of_range("la::vec::vec() : std::initializer_list has incorrect size");

  unsigned long index = 0;
  for (const T& element : list)
    data[index++] = element;
}

template <unsigned long N, typename T>
vec<N, T>::vec(const std::array<T, N>& arr) : data(arr) {}

template <unsigned long N, typename T>
template <unsigned long M>
vec<N, T>::vec(const vec<M, T>& v, std::array<T, N - M> vals)
{
  if (M >= N)
  {
    throw std::out_of_range(
      "la::vec::vec() : attempted to initialize a vector with a vector that was larger or same size"
    );
  }

  for (unsigned long i = 0; i < M; ++i)
    data[i] = v[i];

  for (unsigned long i = 0; i < N - M; ++i)
    data[i + M] = vals[i];
}

template <unsigned long N, typename T>
vec<N, T>& vec<N, T>::operator=(std::initializer_list<T> list)
{
  if (list.size() != N)
    throw std::out_of_range("la::vec::operator= : std::initializer_list has incorrect size");

  unsigned long index = 0;
  for (const T& element : list)
    data[index++] = element;

  return *this;
}

template <unsigned long N, typename T>
T& vec<N, T>::operator [] (unsigned long index)
{
  if (index > N - 1)
    throw std::out_of_range("la::vec::operator[] : index out of range");

  return data[index];
}

template <unsigned long N, typename T>
const T& vec<N, T>::operator [] (unsigned long index) const
{
  if (index > N - 1)
    throw std::out_of_range("la::vec::operator[] : index out of range");

  return data[index];
}

template <unsigned long N, typename T>
vec<N, T> vec<N, T>::operator + (const vec<N, T>& rhs) const
{
  vec<N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = data[i] + rhs[i];

  return result;
}

template <unsigned long N, typename T>
vec<N, T> vec<N, T>::operator - (const vec<N, T>& rhs) const
{
  return *this + -rhs;
}

template <unsigned long N, typename T>
T vec<N, T>::operator * (const vec<N, T>& rhs) const
{
  T result = 0;

  for (unsigned long i = 0; i < N; ++i)
    result += data[i] * rhs[i];

  return result;
}

template <unsigned long N, typename T>
vec<N, T> vec<N, T>::operator / (T rhs) const
{
  vec<N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = data[i] / rhs;

  return result;
}

template <unsigned long N, typename T>
vec<N, T> vec<N, T>::operator - () const
{
  vec<N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = -data[i];

  return result;
}

template <unsigned long N, typename T>
template <unsigned long M>
typename std::enable_if<M == 3 && N == M, vec<M, T>>::type vec<N, T>::operator % (const vec<M, T>& rhs) const
{
  vec<M, T> result;

  for (unsigned long i = 0, j = 1, k = 2; i < N; ++i, j = ++j % 3, k = ++k % 3)
    result[i] = data[j] * rhs[k] - data[k] * rhs[j] * pow(-1, i);

  return result;
}

template <unsigned long N, typename T>
vec<N, T> vec<N, T>::zero()
{
  std::array<T, N> arr = { 0.0 };
  return vec<N, T>(arr);
}

template <unsigned long N, typename T>
T vec<N, T>::norm() const
{
  return sqrt(*this * *this);
}

template <unsigned long N, typename T>
vec<N, T> vec<N, T>::normalized() const
{
  return *this / norm();
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T>::mat()
{
  data = zeros().data;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T>::mat(std::initializer_list<vec<M, T>> list)
{
  fill(list);
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T>& mat<M, N, T>::operator = (std::initializer_list<vec<M, T>> list)
{
  fill(list);
}

template <unsigned long M, unsigned long N, typename T>
vec<M, T>& mat<M, N, T>::operator [] (unsigned long index)
{
  if (index > N - 1)
    throw std::out_of_range("la::mat::col() : index out of range");

  return data[index];
}

template <unsigned long M, unsigned long N, typename T>
const vec<M, T>& mat<M, N, T>::operator [] (unsigned long index) const
{
  if (index > N - 1)
    throw std::out_of_range("la::mat::col() : index out of range");

  return data[index];
}

template <unsigned long M, unsigned long N, typename T>
vec<N, T> mat<M, N, T>::operator () (unsigned long index)
{
  if (index > M - 1)
    throw std::out_of_range("la::mat::row() : index out of range");

  vec<N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = data[i][index];

  return result;
}

template <unsigned long M, unsigned long N, typename T>
const vec<N, T> mat<M, N, T>::operator () (unsigned long index) const
{
  if (index > M - 1)
    throw std::out_of_range("la::mat::row() : index out of range");

  vec<N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = data[i][index];

  return result;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::operator + (const mat<M, N, T>& rhs) const
{
  mat<M, N, T> result;

  for (unsigned long i = 0; i < M; ++i)
    result[i] = (*this)[i] + rhs[i];

  return result;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::operator - (const mat<M, N, T>& rhs) const
{
  return *this + -rhs;
}

template <unsigned long M, unsigned long N, typename T>
template <unsigned long P>
mat<M, P, T> mat<M, N, T>::operator * (const mat<N, P, T>& rhs) const
{
  mat<M, P, T> result;

  for (unsigned long i = 0; i < P; ++i)
  {
    for (unsigned long j = 0; j < M; ++j)
      result[i][j] = (*this)(j) * rhs[i];
  }

  return result;
}

template <unsigned long M, unsigned long N, typename T>
vec<M, T> mat<M, N, T>::operator * (const vec<N, T>& rhs) const
{
  vec<M, T> result;

  for (unsigned long i = 0; i < M; ++i)
    result[i] = (*this)(i) * rhs;

  return result;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::operator / (T rhs) const
{
  mat<M, N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = (*this)[i] / rhs;

  return result;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::operator - () const
{
  mat<M, N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = -(*this)[i];

  return result;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::zeros()
{
  std::array<vec<M, T>, N> zeros_matrix;
  for (unsigned long i = 0; i < N; ++i)
    zeros_matrix[i] = vec<M, T>::zero();

  return mat<M, N, T>(zeros_matrix);
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::identity()
{
  static_assert(M == N, "identity only produces square matrices");

  mat<M, M, T> identity_matrix;

  for (unsigned long i = 0; i < M; ++i)
    identity_matrix[i][i] = 1.0;

  return identity_matrix;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::view_matrix(vec<3, T> ws_r, vec<3, T> ws_t, vec<3, T> ws_y)
{
  static_assert(M == 4 && N == 4, "view_matrix must be type la::mat<4, 4, T>");

  vec<3, T> cs_z = (ws_t - ws_r).normalized();
  vec<3, T> cs_x = (cs_z % ws_y).normalized();
  vec<3, T> cs_y = cs_x % cs_z;

  return mat<4, 4, T>{
    vec<4, T>(cs_x, { -ws_r * cs_x }),
    vec<4, T>(cs_y, { -ws_r * cs_y }),
    vec<4, T>(cs_z, { -ws_r * cs_z }),
    { 0.0, 0.0, 0.0, 1.0 }
  };
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::perspective_projection(T fov_y, T aspect_ratio, T near, T far)
{
  static_assert(M == 4 && N == 4, "perspective_projection must be type la::mat<4, 4, T>");

  T delta_plane = far - near;
  T tan_fov = tan(fov_y / 2);

  return mat<4, 4, T>{
    vec<4, T>{ 1 / (aspect_ratio * tan_fov), 0.0, 0.0, 0.0 },
    vec<4, T>{ 0.0, 1 / tan_fov, 0.0, 0.0 },
    vec<4, T>{ 0.0, 0.0, (far - near) / delta_plane, 1.0 },
    vec<4, T>{ 0.0, 0.0,  -2 * far * near / delta_plane, 0.0 }
  };
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::scale_matrix(T s_x, T s_y, T s_z)
{
  static_assert(M == 4 && N == 4, "scale_matrix must be type la::mat<4, 4, T>");

  return mat<4, 4, T>{
    { s_x, 0.0, 0.0, 0.0 },
    { 0.0, s_y, 0.0, 0.0 },
    { 0.0, 0.0, s_z, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 }
  };
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::translation_matrix(vec<3, T> position)
{
  static_assert(M == 4 && N == 4, "translation_matrix must be type la::mat<4, 4, T>");

  mat<4, 4, T> result = mat<4, 4, T>::identity();
  result[3] = vec<4, T>(position, { 1.0 });

  return result;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::rotation_matrix(T theta, vec<3, T> axis)
{
  static_assert(M == 4 && N == 4, "rotation_matrix must be type la::mat<4, 4, T>");

  auto K = mat<3, 3, T>::cross_product(axis);
  auto R = mat<3, 3, T>::identity() + sin(theta) * K + (1 - cos(theta)) * K * K;   // Rodrigues' Formula

  return mat<4, 4, T>{
    vec<4, T>(R[0], { 0.0 }),
    vec<4, T>(R[1], { 0.0 }),
    vec<4, T>(R[2], { 0.0 }),
    { 0.0, 0.0, 0.0, 1.0 }
  };
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> mat<M, N, T>::cross_product(vec<3, T> v)
{
  static_assert(M == 3 && N == 3, "cross_product must be type la::mat<3, 3, T>");

  return mat<3, 3, T>{
    { 0.0, v[2], -v[1] },
    { -v[2], 0.0, v[0] },
    { v[1], -v[0], 0.0 }
  };
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T>::mat(const std::array<vec<M, T>, N>& arr)
{
  data = arr;
}

template <unsigned long M, unsigned long N, typename T>
void mat<M, N, T>::fill(const std::initializer_list<vec<M, T>>& list)
{
  if (list.size() != N)
    throw std::out_of_range("la::mat::fill() : std::initializer_list has incorrect dimensions");

  unsigned long i = 0;
  for (const vec<M, T>& element : list)
    data[i++] = element;
}

template <unsigned long N, typename T>
vec<N, T> operator * (T lhs, const vec<N, T>& rhs)
{
  vec<N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = lhs * rhs[i];

  return result;
}

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> operator * (T lhs, const mat<M, N, T>& rhs)
{
  mat<M, N, T> result;

  for (unsigned long i = 0; i < N; ++i)
    result[i] = lhs * rhs[i];

  return result;
}

template <typename T>
T radians(T deg)
{
  static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value);

  return deg * M_PI / 180.0;
}

} // namespace la

namespace std
{

template <unsigned long N, typename T>
std::string to_string(const la::vec<N, T>& v)
{
  std::string str = "{ ";

  for (unsigned long i = 0; i < N; ++i)
    str += std::to_string(v[i]) + (i == N - 1 ? " }" : ", ");

  return str;
}

template <unsigned long M, unsigned long N, typename T>
std::string to_string(const la::mat<M, N, T>& m)
{
  std::string str = "";

  for (unsigned long i = 0; i < M; ++i)
  {
    for (unsigned long j = 0; j < N; ++j)
      str += std::to_string(m(i)[j]) + (j == N - 1 ? "\n" : "\t");
  }

  return str;
}

} // namespace std

#endif // str_linalg_templates_hpp
