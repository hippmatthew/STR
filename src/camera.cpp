#include "src/include/camera.hpp"

#include <fstream>

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
      .format   = vk::Format::eR32G32Sfloat,
      .offset   = __offsetof(Vertex, position)
    }
  };
}

Camera::Camera(float np, float fov)
{
  setView();
  adjustNearPlane(np);
  adjustFOV(fov);
}

const la::mat<4>& Camera::view_matrix() const
{
  return view;
}

const la::vec<3>& Camera::near_plane_dimensions() const
{
  return npDims;
}

const vk::raii::Pipeline& Camera::pipeline() const
{
  return vk_pipeline;
}

const vk::raii::PipelineLayout& Camera::pipelineLayout() const
{
  return vk_pipelineLayout;
}

const vk::raii::DescriptorSetLayout& Camera::descriptorLayout() const
{
  return vk_descriptorLayout;
}

const vk::raii::DescriptorSet& Camera::descriptorSet(unsigned long frame) const
{
  return vk_descriptorSets[frame][0];
}

const vk::raii::Buffer& Camera::vertexBuffer() const
{
  return vk_buffers[0];
}

const vk::raii::Buffer& Camera::indexBuffer() const
{
  return vk_buffers[1];
}

void Camera::adjustNearPlane(float np)
{
  npDims[2] = np;
}

void Camera::adjustFOV(float fov)
{
  float height = 2 * npDims[2] * tanf(la::radians(fov) / 2);
  float width = VECS_SETTINGS.aspect_ratio() * height;
  npDims[0] = width;
  npDims[1] = height;
};

void Camera::translate(la::vec<3> displacement)
{
  la::vec<3> normal = { view[2][0], view[2][1], view[2][2] };
  la::vec<3> position = { view[3][0], view[3][1], view[3][2] };
  position = position + displacement;

  setView(position, normal);
}

void Camera::rotate(la::vec<3> angles)
{
  la::vec<3> normal = { view[2][0], view[2][1], view[2][2] };
  la::vec<3> position = { view[3][0], view[3][1], view[3][2] };

  auto Rx = la::mat<4>::rotation_matrix(angles[0], { 1.0, 0.0, 0.0 });
  auto Ry = la::mat<4>::rotation_matrix(angles[1], { 0.0, -1.0, 0.0 });
  auto Rz = la::mat<4>::rotation_matrix(angles[2], { 0.0, 0.0, 1.0 });

  la::vec<4> norm = Rz * Ry * Rx * la::vec<4>(normal, { 1.0 });
  normal = { norm[0], norm[1], norm[2] };

  setView(position, normal);
}

void Camera::load(const vecs::Device& vecs_device, const vecs::GUI& vecs_gui)
{
  loadPipeline(vecs_device, vecs_gui);
  allocateUniforms(vecs_device);
  loadDescriptors(vecs_device);
}

void Camera::updateSSBO(unsigned int frame, const std::vector<Transform>& transforms)
{
  std::array<Transform, STR_MAX_TRANSFORMS> tforms;

  for (unsigned int i = 0; i < transforms.size(); ++i)
    tforms[i] = transforms[i];

  TransformSSBO buffer{
    .size       = static_cast<unsigned int>(transforms.size()),
    .transforms = tforms
  };

  void * memory = vk_memory.mapMemory(offsets[frame + 2], sizeof(TransformSSBO));
  memcpy(memory, &buffer, sizeof(buffer));
  vk_memory.unmapMemory();
}

std::vector<char> Camera::read(std::string path) const
{
  std::vector<char> buffer;

  std::ifstream shader(path, std::ios::ate | std::ios::binary);
  if (shader.fail()) return buffer;

  unsigned long size = shader.tellg();
  buffer.resize(size);

  shader.seekg(0);
  shader.read(buffer.data(), size);

  return buffer;
}

std::array<vk::raii::ShaderModule, 2> Camera::shaderModules(const vecs::Device& vecs_device) const
{
  std::array<vk::raii::ShaderModule, 2> modules = { nullptr, nullptr };
  std::array<std::string, 2> paths = { "shaders/camera.vert.spv", "shaders/camera.frag.spv" };

  for (unsigned int i = 0; i < 2; ++i)
  {
    auto code = read(paths[i]);

    vk::ShaderModuleCreateInfo ci_module{
      .codeSize = code.size(),
      .pCode    = reinterpret_cast<const unsigned int *>(code.data())
    };

    modules[i] = vecs_device.logical().createShaderModule(ci_module);
  }

  return modules;
}

std::array<vk::PipelineShaderStageCreateInfo, 2> Camera::createInfos(const std::array<vk::raii::ShaderModule, 2>& modules) const
{
  std::array<vk::PipelineShaderStageCreateInfo, 2> infos;
  std::array<vk::ShaderStageFlagBits, 2> stages = { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment };

  for (unsigned int i = 0; i < 2; ++i)
  {
    infos[i] = vk::PipelineShaderStageCreateInfo{
      .stage  = stages[i],
      .module = *modules[i],
      .pName  = "main"
    };
  }

  return infos;
}

unsigned int Camera::findIndex(
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

  throw std::runtime_error("error @ str::Camera::findIndex() : could not find suitable memory index");
}

void Camera::setView(la::vec<3> pos, la::vec<3> norm)
{
  view = la::mat<4>::view_matrix(pos, pos + norm, { 0.0, -1.0, 0.0 });
}

void Camera::loadPipeline(const vecs::Device& vecs_device, const vecs::GUI& vecs_gui)
{
  auto modules = shaderModules(vecs_device);
  auto stages = createInfos(modules);

  std::vector<vk::DynamicState> dynamicStates{
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
  };

  vk::PipelineDynamicStateCreateInfo ci_dynamicState{
    .dynamicStateCount  = static_cast<unsigned int>(dynamicStates.size()),
    .pDynamicStates     = dynamicStates.data()
  };

  vk::PipelineViewportStateCreateInfo ci_viewportState{
    .viewportCount  = 1,
    .scissorCount   = 1
  };

  auto binding = Vertex::binding();
  auto attributes = Vertex::attributes();
  vk::PipelineVertexInputStateCreateInfo ci_vertexInput{
    .vertexBindingDescriptionCount    = 1,
    .pVertexBindingDescriptions       = &binding,
    .vertexAttributeDescriptionCount  = static_cast<unsigned int>(attributes.size()),
    .pVertexAttributeDescriptions     = attributes.data()
  };

  vk::PipelineInputAssemblyStateCreateInfo ci_inputAssembly{
    .topology               = vk::PrimitiveTopology::eTriangleList,
    .primitiveRestartEnable = vk::False
  };

  vk::PipelineRasterizationStateCreateInfo ci_rasterizer{
    .depthClampEnable         = vk::False,
    .rasterizerDiscardEnable  = vk::False,
    .polygonMode              = vk::PolygonMode::eFill,
    .cullMode                 = vk::CullModeFlagBits::eNone,
    .frontFace                = vk::FrontFace::eCounterClockwise,
    .depthBiasEnable          = vk::False,
    .depthBiasConstantFactor  = 0.0f,
    .depthBiasClamp           = 0.0f,
    .depthBiasSlopeFactor     = 0.0f,
    .lineWidth                = 1.0f
  };

  vk::PipelineMultisampleStateCreateInfo ci_multisampling{
    .rasterizationSamples   = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable    = vk::False,
    .minSampleShading       = 1.0f,
    .pSampleMask            = nullptr,
    .alphaToCoverageEnable  = vk::False,
    .alphaToOneEnable       = vk::False
  };

  vk::PipelineDepthStencilStateCreateInfo ci_stencil{
    .depthTestEnable  = vk::True,
    .depthWriteEnable = vk::True,
    .depthCompareOp   = vk::CompareOp::eLess,
  };

  vk::PipelineColorBlendAttachmentState blendState{
    .blendEnable          = vk::True,
    .srcColorBlendFactor  = vk::BlendFactor::eSrcAlpha,
    .dstColorBlendFactor  = vk::BlendFactor::eOneMinusSrcAlpha,
    .colorBlendOp         = vk::BlendOp::eAdd,
    .srcAlphaBlendFactor  = vk::BlendFactor::eOne,
    .dstAlphaBlendFactor  = vk::BlendFactor::eZero,
    .alphaBlendOp         = vk::BlendOp::eAdd,
    .colorWriteMask       = vk::ColorComponentFlagBits::eR |
                            vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB |
                            vk::ColorComponentFlagBits::eA
  };

  vk::PipelineColorBlendStateCreateInfo ci_blendState{
    .logicOpEnable    = vk::False,
    .logicOp          = vk::LogicOp::eCopy,
    .attachmentCount  = 1,
    .pAttachments     = &blendState
  };

  vk::DescriptorSetLayoutBinding transformsBinding{
    .binding          = 0,
    .descriptorType   = vk::DescriptorType::eStorageBuffer,
    .descriptorCount  = 1,
    .stageFlags       = vk::ShaderStageFlagBits::eFragment
  };

  vk::DescriptorSetLayoutCreateInfo ci_descriptorLayout{
    .bindingCount = 1,
    .pBindings    = &transformsBinding
  };

  vk_descriptorLayout = vecs_device.logical().createDescriptorSetLayout(ci_descriptorLayout);

  vk::PushConstantRange camera{
    .stageFlags = vk::ShaderStageFlagBits::eVertex,
    .offset     = 0,
    .size       = sizeof(la::mat<4>) + sizeof(la::vec<3>)
  };

  vk::PipelineLayoutCreateInfo ci_pipelineLayout{
    .setLayoutCount         = 1,
    .pSetLayouts            = &*vk_descriptorLayout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges    = &camera
  };

  vk_pipelineLayout = vecs_device.logical().createPipelineLayout(ci_pipelineLayout);

  auto format = VECS_SETTINGS.format();
  auto dformat = VECS_SETTINGS.depth_format();
  vk::PipelineRenderingCreateInfoKHR ci_rendering{
    .colorAttachmentCount     = 1,
    .pColorAttachmentFormats  = &format,
    .depthAttachmentFormat    = dformat
  };

  vk::GraphicsPipelineCreateInfo ci_pipeline{
    .pNext                = &ci_rendering,
    .stageCount           = static_cast<unsigned int>(stages.size()),
    .pStages              = stages.data(),
    .pVertexInputState    = &ci_vertexInput,
    .pInputAssemblyState  = &ci_inputAssembly,
    .pViewportState       = &ci_viewportState,
    .pRasterizationState  = &ci_rasterizer,
    .pMultisampleState    = &ci_multisampling,
    .pDepthStencilState   = &ci_stencil,
    .pColorBlendState     = &ci_blendState,
    .pDynamicState        = &ci_dynamicState,
    .layout               = *vk_pipelineLayout,
  };

  vk_pipeline = vecs_device.logical().createGraphicsPipeline(nullptr, ci_pipeline);
}

void Camera::allocateUniforms(const vecs::Device& vecs_device)
{
  vk::DeviceSize vertexSize = sizeof(Vertex) * 4;
  vk::DeviceSize indexSize = sizeof(unsigned int) * 6;
  vk::DeviceSize ssboSize = sizeof(TransformSSBO);

  vk::BufferCreateInfo ci_vertex{
    .size         = vertexSize,
    .usage        = vk::BufferUsageFlagBits::eVertexBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  };
  vk_buffers.emplace_back(vecs_device.logical().createBuffer(ci_vertex));

  vk::BufferCreateInfo ci_index{
    .size         = indexSize,
    .usage        = vk::BufferUsageFlagBits::eIndexBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  };
  vk_buffers.emplace_back(vecs_device.logical().createBuffer(ci_index));

  vk::BufferCreateInfo ci_ssbo{
    .size         = ssboSize,
    .usage        = vk::BufferUsageFlagBits::eStorageBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  };

  for (unsigned long i = 0; i < VECS_SETTINGS.max_flight_frames(); ++i)
    vk_buffers.emplace_back(vecs_device.logical().createBuffer(ci_ssbo));

  vk::DeviceSize size = 0;
  for (const auto& vk_buffer : vk_buffers)
  {
    auto requirements = vk_buffer.getMemoryRequirements();

    while (size % requirements.alignment != 0)
      ++size;

    offsets.emplace_back(size);
    size += requirements.size;
  }

  vk::MemoryAllocateInfo ai_memory{
    .allocationSize = size,
    .memoryTypeIndex = findIndex(
      vecs_device.physical(),
      vk_buffers[0].getMemoryRequirements().memoryTypeBits,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    )
  };
  vk_memory = vecs_device.logical().allocateMemory(ai_memory);

  unsigned long index = 0;
  for (const auto& vk_buffer : vk_buffers)
    vk_buffer.bindMemory(*vk_memory, offsets[index++]);

  std::array<Vertex, 4> vertices = {
    Vertex{{ -1.0, -1.0 }},
    Vertex{{ -1.0, 1.0 }},
    Vertex{{ 1.0, 1.0 }},
    Vertex{{ 1.0, -1.0 }}
  };

  std::array<unsigned int, 6> indices = { 0, 1, 2, 2, 3, 0 };

  void * memory = vk_memory.mapMemory(0, vertexSize);
  memcpy(memory, vertices.data(), sizeof(vertices));
  vk_memory.unmapMemory();

  memory = vk_memory.mapMemory(offsets[1], indexSize);
  memcpy(memory, indices.data(), sizeof(indices));
  vk_memory.unmapMemory();
}

void Camera::loadDescriptors(const vecs::Device& vecs_device)
{
  vk::DescriptorPoolSize poolSize{
    .type             = vk::DescriptorType::eStorageBuffer,
    .descriptorCount  = static_cast<unsigned int>(VECS_SETTINGS.max_flight_frames())
  };

  vk::DescriptorPoolCreateInfo ci_descriptorPool{
    .flags          = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
    .maxSets        = static_cast<unsigned int>(VECS_SETTINGS.max_flight_frames()),
    .poolSizeCount  = 1,
    .pPoolSizes     = &poolSize
  };

  vk_descriptorPool = vecs_device.logical().createDescriptorPool(ci_descriptorPool);

  std::vector<vk::WriteDescriptorSet> writes;
  for (unsigned long i = 0; i < VECS_SETTINGS.max_flight_frames(); ++i)
  {
    vk::DescriptorSetAllocateInfo ai_descriptors{
      .descriptorPool     = *vk_descriptorPool,
      .descriptorSetCount = 1u,
      .pSetLayouts        = &*vk_descriptorLayout
    };
    vk_descriptorSets.emplace_back(vk::raii::DescriptorSets(vecs_device.logical(), ai_descriptors));

    vk::DescriptorBufferInfo bufferInfo{
      .buffer = *vk_buffers[i + 2],
      .offset = 0,
      .range  = sizeof(TransformSSBO)
    };

    vk::WriteDescriptorSet write{
      .dstSet           = *vk_descriptorSets[i][0],
      .dstBinding       = 0,
      .dstArrayElement  = 0,
      .descriptorCount  = 1,
      .descriptorType   = vk::DescriptorType::eStorageBuffer,
      .pBufferInfo      = &bufferInfo
    };

    writes.emplace_back(write);
  }

  vk::ArrayProxy<vk::WriteDescriptorSet> proxy(writes.size(), writes.data());
  vecs_device.logical().updateDescriptorSets(proxy, nullptr);
}

} // namespace str