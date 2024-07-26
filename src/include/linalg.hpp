#ifndef str_linalg_hpp
#define str_linalg_hpp

#include <array>
#include <initializer_list>
#include <type_traits>
#include <string>

namespace la
{

template <unsigned long N, typename T = float>
class alignas( N == 3 && std::is_same<T, float>::value ? 16 : sizeof(T) * N ) vec
{
  static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value);
  
  public:
    vec();
    vec(const vec&) = default;
    vec(vec&&) = default;
    vec(std::initializer_list<T>);
    
    template<unsigned long M>
    vec(const vec<M, T>&, std::array<T, N - M>);

    ~vec() = default;

    vec& operator = (const vec&) = default;
    vec& operator = (vec&&) = default;
    vec& operator = (std::initializer_list<T>);

    T& operator [] (unsigned long);
    const T& operator [] (unsigned long) const;

    vec operator + (const vec&) const;
    vec operator - (const vec&) const;
    T operator * (const vec&) const;
    vec operator / (T) const;
    vec operator - () const;

    template <unsigned long M>
    typename std::enable_if<M == 3 && N == M, vec<M, T>>::type operator % (const vec<M, T>&) const;

    static vec zero();
    
    T norm() const;
    vec<N, T> normalized() const;
  
  private:
    vec(const std::array<T, N>&);
  
  private:
    std::array<T, N> data;
};

template <unsigned long M, unsigned long N = M, typename T = float>
class alignas( alignof(vec<M, T>) ) mat
{
  static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value);
  
  public:
    mat();
    mat(const mat&) = default;
    mat(mat&&) = default;
    mat(std::initializer_list<vec<M, T>>);

    ~mat() = default;

    mat& operator = (const mat&) = default;
    mat& operator = (mat&&) = default;
    mat& operator = (std::initializer_list<vec<M, T>>);

    vec<M, T>& operator [] (unsigned long index);
    const vec<M, T>& operator [] (unsigned long index) const;

    vec<N, T> operator () (unsigned long index);
    const vec<N, T> operator () (unsigned long index) const;

    mat operator + (const mat&) const;
    mat operator - (const mat&) const;
    
    template <unsigned long P>
    mat<M, P, T> operator * (const mat<N, P, T>&) const;

    vec<M, T> operator * (const vec<N, T>&) const;
    mat operator / (T) const;
    mat operator - () const;

    static mat zeros();
    static mat identity();

    static mat view_matrix(vec<3, T>, vec<3, T>, vec<3, T>);
    static mat perspective_projection(T, T, T, T);

  private:
    mat(const std::array<vec<M, T>, N>&);

    void fill(const std::initializer_list<vec<M, T>>&);
    vec<M, T>& col(unsigned long);
    vec<N, T> row(unsigned long);
  
  private:
    std::array<vec<M, T>, N> data;
};

template <unsigned long N, typename T>
vec<N, T> operator * (T, const vec<N, T>&);

template <unsigned long M, unsigned long N, typename T>
mat<M, N, T> operator * (T, const mat<M, N, T>&);

template <typename T = float>
T radians(T deg);

} // namespace la

namespace std
{

template <unsigned long N, typename T>
std::string to_string(const la::vec<N, T>&);

template <unsigned long M, unsigned long N, typename T>
std::string to_string(const la::mat<M, N, T>&);

} // namespace std

#include "src/include/linalg_templates.hpp"

#endif // str_linalg_hpp