#ifndef str_camera_hpp
#define str_camera_hpp

#include "src/include/transform.hpp"

#include <vecs/vecs.hpp>
#include <vector>

#define STR_MAX_TRANSFORMS 10

namespace str
{

struct TransformSSBO
{
  unsigned int size;
  std::array<Transform, STR_MAX_TRANSFORMS> transforms;
};

struct Vertex
{
  la::vec<2> position;

  static vk::VertexInputBindingDescription binding();
  static std::array<vk::VertexInputAttributeDescription, 1> attributes();
};

class Camera
{
  public:
    Camera(float np = 0.1f, float fov = 60.0f);
    Camera(const Camera&) = delete;
    Camera(Camera&&) = delete;

    ~Camera() = default;

    Camera& operator = (const Camera&) = delete;
    Camera& operator = (Camera&&) = delete;

    const la::mat<4>& view_matrix() const;
    const la::vec<3>& near_plane_dimensions() const;
    const vk::raii::Pipeline& pipeline() const;
    const vk::raii::PipelineLayout& pipelineLayout() const;
    const vk::raii::DescriptorSetLayout& descriptorLayout() const;
    const vk::raii::DescriptorSet& descriptorSet(unsigned long) const;
    const vk::raii::Buffer& vertexBuffer() const;
    const vk::raii::Buffer& indexBuffer() const;

    void adjustNearPlane(float);
    void adjustFOV(float);
    void translate(la::vec<3>);
    void rotate(la::vec<3>);
    void load(const vecs::Device&, const vecs::GUI&);
    void updateSSBO(unsigned int, const std::vector<Transform>&);

  private:
    std::vector<char> read(std::string) const;
    std::array<vk::raii::ShaderModule, 2> shaderModules(const vecs::Device& vecs_device) const;
    std::array<vk::PipelineShaderStageCreateInfo, 2> createInfos(const std::array<vk::raii::ShaderModule, 2>&) const;
    unsigned int findIndex(const vk::raii::PhysicalDevice&, unsigned int, vk::MemoryPropertyFlags) const;

    void setView(la::vec<3> pos = { 0.0, 0.0, 0.0 }, la::vec<3> norm = { 0.0, 0.0, 1.0 });
    void loadPipeline(const vecs::Device&, const vecs::GUI&);
    void allocateUniforms(const vecs::Device&);
    void loadDescriptors(const vecs::Device&);

  private:
    la::vec<3> npDims = la::vec<3>::zero();
    la::mat<4> view = la::mat<4>::view_matrix({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, -1.0, 0.0 });

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

#endif // str_camera_hpp