#ifndef str_material_hpp
#define str_material_hpp

#include "src/include/linalg.hpp"

#include <vecs/vecs.hpp>

#include <memory>
#include <optional>
#include <string>

namespace str
{

struct UniformInfo
{
  unsigned long index;
  unsigned long size;
  vk::DeviceSize offset = 0;
};

class Vertex
{
  public:
    static vk::VertexInputBindingDescription binding();
    static std::array<vk::VertexInputAttributeDescription, 1> attributes();
  
  public:
    la::vec<3> position;
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

        template <typename... Tps>
        MaterialBuilder& uniforms();

      private:
        template <typename T>
        void uniform();
      
      private:
        std::vector<std::pair<vk::ShaderStageFlagBits, std::string>> paths;
        std::map<const char *, unsigned long> indexMap;
        std::vector<UniformInfo> uniformInfo;
    };
  
  friend class MaterialBuilder;
  
  public:
    Material() = default;
    Material(const Material&) = delete;
    Material(Material&&) = delete;
    Material(const MaterialBuilder&);

    ~Material() = default;

    Material& operator = (const Material&) = delete;
    Material& operator = (Material&&) = delete;
    void operator = (const MaterialBuilder&);

    static MaterialBuilder Builder();

    const vk::raii::Pipeline& pipeline() const;
    const vk::raii::PipelineLayout& pipelineLayout() const;
    const vk::raii::DescriptorSetLayout& descriptorLayout() const;
    std::vector<vk::DescriptorSet> descriptorSets(unsigned long) const;

    void load(const vecs::Device&);
    
    template <typename... Tps>
    void updateUniforms(Tps&...);
    
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

    void loadPipeline(const vecs::Device&);
    void allocateUniforms(const vecs::Device&);
    void loadDescriptors(const vecs::Device&);

    template <typename T>
    void updateUniform(T&);
  
  private:
    std::vector<std::pair<vk::ShaderStageFlagBits, std::string>> paths;
    std::map<const char *, unsigned long> indexMap;
    std::vector<UniformInfo> uniformInfo;
    
    vk::raii::DescriptorSetLayout vk_descriptorLayout = nullptr;
    vk::raii::PipelineLayout vk_pipelineLayout = nullptr;
    vk::raii::Pipeline vk_pipeline = nullptr;

    vk::raii::DeviceMemory vk_memory = nullptr;
    std::vector<vk::raii::Buffer> vk_uniforms;

    vk::raii::DescriptorPool vk_descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSets> vk_descriptorSets;
};

} // namespace str

#include "src/include/material_templates.hpp"

#endif // str_material_hpp