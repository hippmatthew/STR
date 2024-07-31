#include "src/include/material.hpp"
#include "src/include/vertex.hpp"

#include <iostream>
#include <fstream>
#include <vulkan/vulkan.hpp>

namespace str
{

Material::MaterialBuilder& Material::MaterialBuilder::shader(vk::ShaderStageFlagBits stage, std::string path)
{
  paths.emplace_back(std::make_pair(stage, path));
  return *this;
}

Material::Material(const Material& mat)
{
  paths = mat.paths;
}

Material::Material(Material&& mat)
{
  paths = std::move(mat.paths);
}

Material::Material(const MaterialBuilder& builder)
{
  paths = builder.paths;
}

Material& Material::operator = (const Material& mat)
{
  paths = mat.paths;
  return *this;
}

Material& Material::operator = (Material&& mat)
{
  paths = std::move(mat.paths);
  return *this;
}

Material& Material::operator = (const MaterialBuilder& builder)
{
  paths = builder.paths;
  return *this;
}

Material::MaterialBuilder Material::Builder()
{
  return MaterialBuilder();
}

const vk::raii::Pipeline& Material::pipeline() const
{
  return vk_pipeline;
}

const vk::raii::PipelineLayout& Material::pipelineLayout() const
{
  return vk_pipelineLayout;
}

const vk::raii::DescriptorSetLayout& Material::descriptorLayout() const
{
  return vk_descriptorLayout;
}

const vk::raii::DescriptorSet& Material::descriptorSet(unsigned long frame) const
{
  return vk_descriptorSets[frame][0];
}

std::vector<char> Material::read(const std::string& path) const
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

void Material::load(const vecs::Device& vecs_device, const vecs::GUI& vecs_gui)
{
  loadPipeline(vecs_device, vecs_gui);
  allocateUniforms(vecs_device);
  loadDescriptors(vecs_device);
}

void Material::updateTransforms(unsigned long frame, const std::vector<Transform>& ts)
{
  std::array<Transform, STR_MAX_OBJECTS> transforms;
  for (unsigned long i = 0; i < (STR_MAX_OBJECTS < ts.size() ? STR_MAX_OBJECTS : ts.size()); ++i)
    transforms[i] = ts[i];

  TransformBuffer buffer{
    .count      = static_cast<unsigned int>(ts.size()),
    .transforms = transforms
  };

  void * memory = vk_memory.mapMemory(offsets[frame], sizeof(TransformBuffer));
  memcpy(memory, &buffer, sizeof(buffer));
  vk_memory.unmapMemory();
}

std::vector<std::pair<vk::ShaderStageFlagBits, vk::raii::ShaderModule>> Material::shaderModules(const vecs::Device& vecs_device) const
{
  std::vector<std::pair<vk::ShaderStageFlagBits, vk::raii::ShaderModule>> modules;

  for (const auto& path : paths)
  {
    auto code = read(path.second);

    vk::ShaderModuleCreateInfo ci_module{
      .codeSize = code.size(),
      .pCode    = reinterpret_cast<const unsigned int *>(code.data())
    };

    modules.emplace_back(std::make_pair(path.first, vecs_device.logical().createShaderModule(ci_module)));
  }

  return modules;
}

std::vector<vk::PipelineShaderStageCreateInfo> Material::createInfos(
  const std::vector<std::pair<vk::ShaderStageFlagBits, vk::raii::ShaderModule>>& modules
) const
{
  std::vector<vk::PipelineShaderStageCreateInfo> infos;

  for (auto& module : modules)
  {
    vk::PipelineShaderStageCreateInfo ci_stage{
      .stage  = module.first,
      .module = *module.second,
      .pName  = "main"
    };

    infos.emplace_back(ci_stage);
  }

  return infos;
}

unsigned int Material::findIndex(
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

void Material::loadPipeline(const vecs::Device& vecs_device, const vecs::GUI& vecs_gui)
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

void Material::allocateUniforms(const vecs::Device& vecs_device)
{
  vk::DeviceSize size = sizeof(TransformBuffer);

  vk::BufferCreateInfo ci_buffer{
    .size         = size,
    .usage        = vk::BufferUsageFlagBits::eStorageBuffer,
    .sharingMode  = vk::SharingMode::eExclusive
  };

  for (unsigned long i = 0; i < VECS_SETTINGS.max_flight_frames(); ++i)
    vk_buffers.emplace_back(vecs_device.logical().createBuffer(ci_buffer));

  size = 0;
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
}

void Material::loadDescriptors(const vecs::Device& vecs_device)
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
      .buffer = *vk_buffers[i],
      .offset = 0,
      .range  = sizeof(TransformBuffer)
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