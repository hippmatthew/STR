#include "src/include/vertex.hpp"
#include <sys/_types.h>
#include <vk_video/vulkan_video_codec_h264std.h>

namespace str
{

vk::VertexInputBindingDescription Vertex::binding()
{
  return vk::VertexInputBindingDescription{
    .binding    = 0,
    .stride     = sizeof(Vertex),
    .inputRate  = vk::VertexInputRate::eVertex
  };
}

std::array<vk::VertexInputAttributeDescription, 1> Vertex::attributes()
{
  return std::array<vk::VertexInputAttributeDescription, 1>{
    vk::VertexInputAttributeDescription{
      .location = 0,
      .binding  = 0,
      .format   = vk::Format::eR32G32B32Sfloat,
      .offset   = __offsetof(Vertex, position)
    }
  };
}

} // namespace str