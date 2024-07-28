#ifndef str_engine_hpp
#define str_engine_hpp

#include "src/include/renderer.hpp"

#include <vecs/vecs.hpp>

namespace str
{

class Engine : public vecs::Engine
{
  public:
    ~Engine();

    void load() override;
    void run() override;

  private:
    void loadMaterials();
    void setupECS();
    void loadComponents();

  private:
    std::shared_ptr<Renderer> renderer;
    std::array<std::shared_ptr<Material>, 2> materials;

    float time = 0.0f;
    float prevTime = 0.0f;
    float delta_time = 0.0f;
};

} // namespace str

#endif // str_engine_hpp