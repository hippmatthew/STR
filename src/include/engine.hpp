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
    void setupECS();
    void loadComponents();

  private:
    float time = 0.0f;
    float prevTime = 0.0f;
    float delta_time = 0.0f;

    std::vector<Transform> transforms;

    std::shared_ptr<Renderer> renderer;
};

} // namespace str

#endif // str_engine_hpp