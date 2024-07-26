#ifndef str_renderer_hpp
#define str_renderer_hpp

#include "src/include/graphics.hpp"
#include "src/include/camera.hpp"

#include <vecs/vecs.hpp>

#include <map>
#include <string>
#include <vector>

#define P_GRAPHICS std::shared_ptr<str::Graphics>

namespace str
{

class Renderer : public vecs::System
{
  public:
    Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    ~Renderer() = default;

    Renderer& operator = (const Renderer&) = delete;
    Renderer& operator = (Renderer&&) = delete;

    void update(const std::shared_ptr<vecs::ComponentManager>&, std::set<unsigned long>) override;
    
    void waitFlight() const;
    
    void link(std::shared_ptr<vecs::Device>, std::shared_ptr<vecs::GUI>);
    void initialize();
    
  private:
    void checkResult(const vk::Result&, std::string) const;

    void begin(unsigned int);
    void render(const std::shared_ptr<vecs::ComponentManager>&, unsigned long, unsigned int);
    void end(unsigned int);
  
  private:
    unsigned int frame = 0;
    Camera camera;
    
    std::vector<vk::raii::Fence> flightFences;
    std::vector<vk::raii::Semaphore> imageSemaphores;
    std::vector<vk::raii::Semaphore> renderSemaphores;
    
    std::shared_ptr<vecs::GUI> vecs_gui;
    std::shared_ptr<vecs::Device> vecs_device;
    
    vk::raii::CommandPool vk_commandPool = nullptr;
    vk::raii::CommandBuffers vk_commandBuffers = nullptr;
};

} // namespace str

#endif // str_renderer_hpp