#ifndef str_material_hpp
#define str_material_hpp

#include "src/include/transform.hpp"

#include <vecs/vecs.hpp>

#include <string>

#define STR_MAX_OBJECTS 10u

namespace str
{

struct TransformBuffer{
  unsigned int count;
  std::array<Transform, STR_MAX_OBJECTS> transforms;
};

class Material
{
  private:
    class MaterialBuilder
    {
      friend class Material;

      public:
        MaterialBuilder() = default;
        MaterialBuilder(const MaterialBuilder&) = delete;
        MaterialBuilder(MaterialBuilder&&) = delete;

        ~MaterialBuilder() = default;

        MaterialBuilder& operator = (const MaterialBuilder&) = delete;
        MaterialBuilder& operator = (MaterialBuilder&&) = delete;

        MaterialBuilder& shader(vk::ShaderStageFlagBits, std::string);

      private:
        std::vector<std::pair<vk::ShaderStageFlagBits, std::string>> paths;
    };

  public:
    Material() = default;
    Material(const Material&);
    Material(Material&&);
    Material(const MaterialBuilder&);

    ~Material() = default;

    Material& operator = (const Material&);
    Material& operator = (Material&&);
    Material& operator = (const MaterialBuilder&);

    static MaterialBuilder Builder();

    const vk::raii::Pipeline& pipeline() const;
    const vk::raii::PipelineLayout& pipelineLayout() const;
    const vk::raii::DescriptorSetLayout& descriptorLayout() const;
    const vk::raii::DescriptorSet& descriptorSet(unsigned long) const;

    void load(const vecs::Device&, const vecs::GUI&);
    void updateTransforms(unsigned long, const std::vector<Transform>&);

  private:
    std::vector<char> read(const std::string&) const;
    std::vector<std::pair<vk::ShaderStageFlagBits, vk::raii::ShaderModule>> shaderModules(const vecs::Device&) const;

    std::vector<vk::PipelineShaderStageCreateInfo> createInfos(
      const std::vector<std::pair<vk::ShaderStageFlagBits, vk::raii::ShaderModule>>&
    ) const;

    unsigned int findIndex(
      const vk::raii::PhysicalDevice&,
      unsigned int,
      vk::MemoryPropertyFlags
    ) const;

    void loadPipeline(const vecs::Device&, const vecs::GUI&);
    void allocateUniforms(const vecs::Device&);
    void loadDescriptors(const vecs::Device&);

  private:
    std::vector<std::pair<vk::ShaderStageFlagBits, std::string>> paths;

    vk::raii::DescriptorSetLayout vk_descriptorLayout = nullptr;
    vk::raii::PipelineLayout vk_pipelineLayout = nullptr;
    vk::raii::Pipeline vk_pipeline = nullptr;

    vk::raii::DeviceMemory vk_memory = nullptr;
    std::vector<vk::raii::Buffer> vk_buffers;
    std::vector<vk::DeviceSize> offsets;

    vk::raii::DescriptorPool vk_descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSets> vk_descriptorSets;
};

} // namespace str

#endif // str_material_hpp