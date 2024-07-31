#ifndef str_engine_hpp
#define str_engine_hpp

#include "src/include/renderer.hpp"

#include <vecs/vecs.hpp>

#define SAMPLE_SIZE 50

namespace str
{

class Engine : public vecs::Engine
{
  public:
    ~Engine();

    void load() override;
    void run() override;

  private:
    float average() const;

    void setupECS();
    void loadComponents();


  private:
    float delta_time = 0.0f;
    std::array<float, SAMPLE_SIZE> timings;
    unsigned long index = 0;

    std::vector<Transform> transforms;

    std::shared_ptr<Renderer> renderer;
};

} // namespace str

#endif // str_engine_hpp