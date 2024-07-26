#ifndef str_engine_hpp
#define str_engine_hpp

#include "src/include/renderer.hpp"

#include <vecs/vecs.hpp>

#include <chrono>

namespace str
{

struct Color
{
  la::vec<3> value;
};

class Engine : public vecs::Engine
{
  public:
    ~Engine();
    
    void load() override;
    void run() override;

  private:
    bool close_condition() override;
  
  private:
    std::shared_ptr<Material> material;
    std::shared_ptr<Renderer> renderer;
    Color color{{0.0, 1.0, 0.0}};
    float time = 0.0;

    unsigned int loopCounter = 0;
};

} // namespace str

#endif // str_engine_hpp