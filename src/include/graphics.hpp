#ifndef str_graphics_hpp
#define str_graphics_hpp

#include "src/include/material.hpp"
#include "src/include/linalg.hpp"

#include <vecs/vecs.hpp>

namespace str
{

class Graphics
{
  private:
    class GraphicsBuilder
    {
      friend class Graphics;

      public:
        GraphicsBuilder() = default;
        GraphicsBuilder(const GraphicsBuilder&) = delete;
        GraphicsBuilder(GraphicsBuilder&&) = delete;

        ~GraphicsBuilder() = default;

        GraphicsBuilder& operator = (const GraphicsBuilder&) = delete;
        GraphicsBuilder& operator = (GraphicsBuilder&&) = delete;

        GraphicsBuilder& vertices(const std::vector<Vertex>);
        GraphicsBuilder& indices(const std::vector<unsigned int>);
        GraphicsBuilder& material(std::shared_ptr<Material>);

      private:
        std::vector<Vertex> verts;
        std::vector<unsigned int> inds;
        std::shared_ptr<Material> mat;
    };

  friend class GraphicsBuilder;

  public:
    Graphics() = default;
    Graphics(const Graphics&);
    Graphics(Graphics&&);
    Graphics(const GraphicsBuilder&);

    ~Graphics() = default;

    Graphics& operator = (const Graphics&);
    Graphics& operator = (Graphics&&);
    Graphics& operator = (const GraphicsBuilder&);

    static GraphicsBuilder Builder();

    const vk::raii::Buffer& vertexBuffer() const;
    const vk::raii::Buffer& indexBuffer() const;
    std::shared_ptr<Material> material() const;
    const unsigned long indexCount() const;

    void initialize(const vecs::Device&);

  private:
    unsigned int findIndex(
      const vk::raii::PhysicalDevice&,
      unsigned int,
      vk::MemoryPropertyFlags
    ) const;

  private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> p_material;

    vk::raii::DeviceMemory vk_memory = nullptr;
    vk::raii::Buffer vk_vertexBuffer = nullptr;
    vk::raii::Buffer vk_indexBuffer = nullptr;
};

};

#endif // str_graphics_hpp