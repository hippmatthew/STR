#include "src/include/material.hpp"

#include <fstream>

#include <glm/glm.hpp>

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

Material::MaterialBuilder& Material::MaterialBuilder::shader(vk::ShaderStageFlagBits stage, std::string path)
{
  paths.emplace_back(std::make_pair(stage, path));
  return *this;
}

Material::Material(const MaterialBuilder& builder)
{
  paths = builder.paths;
  indexMap = builder.indexMap;
  uniformInfo = builder.uniformInfo;
}

void Material::operator = (const MaterialBuilder& builder)
{
  paths = builder.paths;
  indexMap = builder.indexMap;
  uniformInfo = builder.uniformInfo;
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

std::vector<vk::DescriptorSet> Material::descriptorSets(unsigned long frame) const
{
  std::vector<vk::DescriptorSet> sets;
  for (const auto& set : vk_descriptorSets[frame])
    sets.emplace_back(*set);

  return sets;
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

void Material::load(const vecs::Device& vecs_device)
{
  loadPipeline(vecs_device);
  allocateUniforms(vecs_device);
  loadDescriptors(vecs_device);
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

void Material::loadPipeline(const vecs::Device& vecs_device)
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

  std::vector<vk::DescriptorSetLayoutBinding> uniformBindings;
  for (unsigned int i = 0; i < uniformInfo.size(); ++i)
  {
    vk::DescriptorSetLayoutBinding uniformBinding{
      .binding          = static_cast<unsigned int>(uniformInfo[i].index),
      .descriptorType   = vk::DescriptorType::eUniformBuffer,
      .descriptorCount  = 1,
      .stageFlags       = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
    };
    uniformBindings.emplace_back(uniformBinding);
  }

  vk::DescriptorSetLayoutCreateInfo ci_descriptorLayout{
    .bindingCount = static_cast<unsigned int>(uniformBindings.size()),
    .pBindings    = uniformBindings.data()
  };

  vk_descriptorLayout = vecs_device.logical().createDescriptorSetLayout(ci_descriptorLayout);

  vk::PushConstantRange mvpMatrixRange{
    .stageFlags = vk::ShaderStageFlagBits::eVertex,
    .offset     = 0,
    .size       = sizeof(la::mat<4>)
  };

  vk::PipelineLayoutCreateInfo ci_pipelineLayout{
    .setLayoutCount         = 1,
    .pSetLayouts            = &*vk_descriptorLayout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges    = &mvpMatrixRange
  };

  vk_pipelineLayout = vecs_device.logical().createPipelineLayout(ci_pipelineLayout);

  auto format = VECS_SETTINGS.format();
  vk::PipelineRenderingCreateInfoKHR ci_rendering{
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats  = &format
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
    .pDepthStencilState   = nullptr,
    .pColorBlendState     = &ci_blendState,
    .pDynamicState        = &ci_dynamicState,
    .layout               = *vk_pipelineLayout,
  };

  vk_pipeline = vecs_device.logical().createGraphicsPipeline(nullptr, ci_pipeline);
}

void Material::allocateUniforms(const vecs::Device& vecs_device)
{
  vk::DeviceSize size = 0;
  unsigned int filter = 0;
  for (unsigned long i = 0; i < uniformInfo.size(); ++i)
  {
    UniformInfo info = uniformInfo[i];

    vk::BufferCreateInfo ci_buffer{
      .size         = static_cast<unsigned int>(info.size),
      .usage        = vk::BufferUsageFlagBits::eUniformBuffer,
      .sharingMode  = vk::SharingMode::eExclusive
    };
    vk_uniforms.emplace_back(vecs_device.logical().createBuffer(ci_buffer));

    auto requirements = vk_uniforms.back().getMemoryRequirements();
    
    while (size % requirements.alignment != 0)
      ++size;

    info.offset = size;
    uniformInfo[i] = info;

    size += requirements.size;
    filter |= requirements.memoryTypeBits;
  }

  vk::MemoryAllocateInfo ai_memory{
    .allocationSize = size,
    .memoryTypeIndex = findIndex(
      vecs_device.physical(),
      filter,
      vk::MemoryPropertyFlagBits::eHostVisible |vk::MemoryPropertyFlagBits::eHostCoherent
    )
  };
  vk_memory = vecs_device.logical().allocateMemory(ai_memory);

  for (unsigned long i = 0; i < uniformInfo.size(); ++i)
    vk_uniforms[i].bindMemory(*vk_memory, uniformInfo[i].offset);
} 

void Material::loadDescriptors(const vecs::Device& vecs_device)
{
  vk::DescriptorPoolSize poolSize{
    .type             = vk::DescriptorType::eUniformBuffer,
    .descriptorCount  = static_cast<unsigned int>(VECS_SETTINGS.max_flight_frames() * uniformInfo.size())
  };

  vk::DescriptorPoolCreateInfo ci_descriptorPool{
    .flags          = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
    .maxSets        = static_cast<unsigned int>(VECS_SETTINGS.max_flight_frames() * uniformInfo.size()),
    .poolSizeCount  = 1,
    .pPoolSizes     = &poolSize
  };

  vk_descriptorPool = vecs_device.logical().createDescriptorPool(ci_descriptorPool);

  std::vector<vk::WriteDescriptorSet> writes;

  for (unsigned long i = 0; i < VECS_SETTINGS.max_flight_frames(); ++i)
  {
    vk::DescriptorSetAllocateInfo ai_descriptorSets{
      .descriptorPool     = *vk_descriptorPool,
      .descriptorSetCount = static_cast<unsigned int>(uniformInfo.size()),
      .pSetLayouts        = &*vk_descriptorLayout
    };
    vk_descriptorSets.emplace_back(vk::raii::DescriptorSets(vecs_device.logical(), ai_descriptorSets));

    for (unsigned long j = 0; j < uniformInfo.size(); ++j)
    {
      vk::DescriptorBufferInfo bufferInfo{
        .buffer = *vk_uniforms[j],
        .offset = 0,
        .range  = static_cast<unsigned int>(uniformInfo[j].size)
      };

      vk::WriteDescriptorSet write{
        .dstSet           = *vk_descriptorSets[i][j],
        .dstBinding       = static_cast<unsigned int>(uniformInfo[j].index),
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo      = &bufferInfo
      };

      writes.emplace_back(write);
    }
  }

  vk::ArrayProxy<vk::WriteDescriptorSet> proxy(writes.size(), writes.data());
  vecs_device.logical().updateDescriptorSets(proxy, nullptr);
}

} // namespace str