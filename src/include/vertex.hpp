#ifndef str_vertex_hpp
#define str_vertex_hpp

#include "src/include/linalg.hpp"

#include <vecs/vecs.hpp>

namespace str
{

class Vertex
{
  public:
    static vk::VertexInputBindingDescription binding();
    static std::array<vk::VertexInputAttributeDescription, 1> attributes();

  public:
    la::vec<3> position;
};

} // namespace str

#endif // str_vertex_hpp