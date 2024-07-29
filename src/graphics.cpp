#include "src/include/graphics.hpp"

namespace str
{

Graphics::GraphicsBuilder& Graphics::GraphicsBuilder::vertices(std::vector<Vertex> v)
{
  verts = v;
  return *this;
}

Graphics::GraphicsBuilder& Graphics::GraphicsBuilder::indices(std::vector<unsigned int> i)
{
  inds = i;
  return *this;
}

Graphics::Graphics(const Graphics& graphics)
{
  vertices = graphics.vertices;
  indices = graphics.indices;
}

Graphics::Graphics(Graphics&& graphics)
{
  vertices = std::move(graphics.vertices);
  indices = std::move(graphics.indices);
}

Graphics::Graphics(const GraphicsBuilder& builder)
{
  vertices = std::move(builder.verts);
  indices = std::move(builder.inds);
}

Graphics& Graphics::operator = (const Graphics& graphics)
{
  if (this == &graphics) return *this;

  vertices = graphics.vertices;
  indices = graphics.indices;

  return *this;
}

Graphics& Graphics::operator = (Graphics&& graphics)
{
  if (this == &graphics) return *this;

  vertices = std::move(graphics.vertices);
  indices = std::move(graphics.indices);

  return *this;
}

Graphics& Graphics::operator = (const GraphicsBuilder& builder)
{
  vertices = std::move(builder.verts);
  indices = std::move(builder.inds);

  return *this;
}

Graphics::GraphicsBuilder Graphics::Builder()
{
  return GraphicsBuilder();
}

const vk::raii::Buffer& Graphics::vertexBuffer() const
{
  return vk_vertexBuffer;
}

const vk::raii::Buffer& Graphics::indexBuffer() const
{
  return vk_indexBuffer;
}

const unsigned long Graphics::indexCount() const
{
  return indices.size();
}

void Graphics::initialize(const vecs::Device& vecs_device)
{
  vk::DeviceSize vertexSize = sizeof(Vertex) * vertices.size();
  vk::DeviceSize indexSize = sizeof(unsigned int) * indices.size();

  vk::BufferCreateInfo ci_vertexBuffer{
    .size         = vertexSize,
    .usage        = vk::BufferUsageFlagBits::eTransferSrc,
    .sharingMode  = vk::SharingMode::eExclusive
  };

  vk::BufferCreateInfo ci_indexBuffer{
    .size         = indexSize,
    .usage        = vk::BufferUsageFlagBits::eTransferSrc,
    .sharingMode  = vk::SharingMode::eExclusive
  };

  auto vertexTransfer = vecs_device.logical().createBuffer(ci_vertexBuffer);
  auto indexTransfer = vecs_device.logical().createBuffer(ci_indexBuffer);

  auto vertexRequirements = vertexTransfer.getMemoryRequirements();
  auto indexRequirements = indexTransfer.getMemoryRequirements();

  auto offset = vertexRequirements.size;
  while (offset % indexRequirements.alignment != 0)
    ++offset;

  vk::MemoryAllocateInfo ai_memory{
    .allocationSize = offset + indexRequirements.size,
    .memoryTypeIndex = findIndex(
      vecs_device.physical(),
      vertexRequirements.memoryTypeBits | indexRequirements.memoryTypeBits,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    )
  };

  auto transferMemory = vecs_device.logical().allocateMemory(ai_memory);

  vertexTransfer.bindMemory(*transferMemory, 0);
  indexTransfer.bindMemory(*transferMemory, offset);

  void * memory = transferMemory.mapMemory(0, vertexSize);
  memcpy(memory, vertices.data(), vertexSize);
  transferMemory.unmapMemory();

  memory = transferMemory.mapMemory(offset, indexSize);
  memcpy(memory, indices.data(), indexSize);
  transferMemory.unmapMemory();

  ci_vertexBuffer.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
  ci_indexBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;

  vk_vertexBuffer = vecs_device.logical().createBuffer(ci_vertexBuffer);
  vk_indexBuffer = vecs_device.logical().createBuffer(ci_indexBuffer);

  vertexRequirements = vk_vertexBuffer.getMemoryRequirements();
  indexRequirements = vk_indexBuffer.getMemoryRequirements();

  offset = vertexRequirements.size;
  while (offset % indexRequirements.alignment != 0)
    ++offset;

  ai_memory.allocationSize = offset + indexRequirements.size;
  ai_memory.memoryTypeIndex = findIndex(
    vecs_device.physical(),
    vertexRequirements.memoryTypeBits | indexRequirements.memoryTypeBits,
    vk::MemoryPropertyFlagBits::eDeviceLocal
  );

  vk_memory = vecs_device.logical().allocateMemory(ai_memory);
  vk_vertexBuffer.bindMemory(*vk_memory, 0);
  vk_indexBuffer.bindMemory(*vk_memory, offset);

  auto family = vecs::FamilyType::All;
  if (vecs_device.hasFamily(vecs::FamilyType::Transfer))
    family = vecs::FamilyType::Transfer;
  else if (vecs_device.hasFamily(vecs::FamilyType::Async))
    family = vecs::FamilyType::Async;

  vk::CommandPoolCreateInfo ci_commandPool{
    .flags            = vk::CommandPoolCreateFlagBits::eTransient,
    .queueFamilyIndex = static_cast<unsigned int>(vecs_device.familyIndex(family))
  };
  auto transferPool = vecs_device.logical().createCommandPool(ci_commandPool);

  vk::CommandBufferAllocateInfo ci_transferBuffers{
    .commandPool        = *transferPool,
    .commandBufferCount = 2
  };
  auto transferBuffers = vecs_device.logical().allocateCommandBuffers(ci_transferBuffers);

  vk::CommandBufferBeginInfo beginTransfer;

  for (auto& transferBuffer : transferBuffers)
    transferBuffer.begin(beginTransfer);

  vk::BufferCopy vertexCopy{
    .srcOffset  = 0,
    .dstOffset  = 0,
    .size       = vertexSize
  };

  vk::BufferCopy indexCopy{
    .srcOffset  = 0,
    .dstOffset  = 0,
    .size       = indexSize
  };

  transferBuffers[0].copyBuffer(*vertexTransfer, *vk_vertexBuffer, vertexCopy);
  transferBuffers[0].copyBuffer(*indexTransfer, *vk_indexBuffer, indexCopy);

  for (auto& transferBuffer : transferBuffers)
    transferBuffer.end();

  vk::FenceCreateInfo ci_fence{};
  auto transferFence = vecs_device.logical().createFence(ci_fence);

  std::vector<vk::CommandBuffer> submitBuffers;
  for (auto& transferBuffer : transferBuffers)
    submitBuffers.emplace_back(*transferBuffer);

  vk::SubmitInfo submitInfo{
    .commandBufferCount = static_cast<unsigned int>(submitBuffers.size()),
    .pCommandBuffers    = submitBuffers.data()
  };

  vecs_device.queue(family).submit(submitInfo, *transferFence);

  static_cast<void>(vecs_device.logical().waitForFences(*transferFence, vk::True, UINT64_MAX));
}

unsigned int Graphics::findIndex(
  const vk::raii::PhysicalDevice& vk_physicalDevice,
  unsigned int filter,
  vk::MemoryPropertyFlags flags
) const
{
  auto properties = vk_physicalDevice.getMemoryProperties();

  for (unsigned long i = 0; i < properties.memoryTypeCount; ++i)
  {
    if ((filter & (1 << i)) &&
        (properties.memoryTypes[i].propertyFlags & flags) == flags)
    {
      return i;
    }
  }

  throw std::runtime_error("error @ str::Graphics::findIndex() : could not find suitable memory index");
}

} // namespace str