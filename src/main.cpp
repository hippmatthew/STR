#include "src/include/engine.hpp"

int main()
{
  VECS_SETTINGS.add_device_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

  str::Engine engine;

  engine.load();
  engine.run();
}