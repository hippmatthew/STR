#include "src/include/engine.hpp"
#include "src/include/linalg.hpp"
#include "src/include/renderer.hpp"
#include "src/include/transform.hpp"

#include <chrono>

namespace str
{

Engine::~Engine()
{
  for (auto& material : materials)
    material.reset();

  renderer.reset();

  component_manager->unregister_components<P_GRAPHICS>();
  system_manager->erase<Renderer>();
}

void Engine::load()
{
  loadMaterials();

  vk::PhysicalDeviceDynamicRenderingFeatures dynamicRendering{
    .dynamicRendering = vk::True
  };

  initialize(&dynamicRendering);

  setupECS();
  loadComponents();
}

void Engine::run()
{
  float da = la::radians(120.0f);

  while (!close_condition())
  {
    poll_gui();
    renderer->waitFlight();

    auto start_frame = std::chrono::high_resolution_clock::now();

    renderer->update(component_manager, { 0 });

    auto t = fmod(da * time, 2 * M_PI);
    float a = 0.5 * (1.0 + cos(t));
    float b = 0.5 * (1.0 + cos(t - da));
    float c = 0.5 * (1.0 + cos(t - 2 * da));

    component_manager->update_data<Color>(0, Color{{ b, a, c }});

    auto transform = component_manager->retrieve<Transform>(0).value();
    transform.rotate({ 0.0f, la::radians(30.0f) * delta_time, 0.0f });
    component_manager->update_data(0, transform);

    auto color = component_manager->retrieve<Color>(0).value();
    materials[0]->updateUniforms(color);

    auto end_frame = std::chrono::high_resolution_clock::now();

    prevTime = time;
    time += std::chrono::duration<float, std::chrono::seconds::period>(end_frame - start_frame).count();
    delta_time = time - prevTime;
  }

  vecs_device->logical().waitIdle();
}

void Engine::loadMaterials()
{
  materials[0] = std::make_shared<Material>(
    Material::Builder()
      .shader(vk::ShaderStageFlagBits::eVertex, "spv/shader.vert.spv")
      .shader(vk::ShaderStageFlagBits::eFragment, "spv/shader.frag.spv")
      .uniforms<Color>()
  );
}

void Engine::setupECS()
{
  entity_manager->new_entity();

  component_manager->register_components<P_GRAPHICS, Transform, Color>();

  system_manager->emplace<Renderer>();
  system_manager->add_components<Renderer, P_GRAPHICS, Transform>();
  renderer = system_manager->system<Renderer>().value();

  entity_manager->add_components<P_GRAPHICS, Transform, Color>(0);

  P_GRAPHICS graphics = std::make_shared<Graphics>(
    Graphics::Builder()
      .vertices({
        {{ 0.5, 0.5, 0.5 }},
        {{ 0.5, 0.5, -0.5 }},
        {{ 0.5, -0.5, 0.5 }},
        {{ 0.5, -0.5, -0.5 }},
        {{ -0.5, 0.5, 0.5 }},
        {{ -0.5, 0.5, -0.5 }},
        {{ -0.5, -0.5, 0.5 }},
        {{ -0.5, -0.5, -0.5 }}
      })
      .indices({
        0, 2, 3, 3, 1, 0,
        4, 5, 7, 7, 6, 4,
        1, 5, 4, 4, 0, 1,
        3, 2, 6, 6, 7, 3,
        0, 4, 6, 6, 2, 0,
        1, 3, 7, 7, 5, 1
      })
      .material(materials[0])
  );

  component_manager->update_data(0,
    graphics,
    Transform{}
      .scale({ 0.5, 0.5, 0.5 })
      .translate(5.0, { 0.0, 0.0, 1.0 }),
    Color{{0.0, 1.0, 0.0}}
  );
}

void Engine::loadComponents()
{
  component_manager->retrieve<P_GRAPHICS>(0).value()->initialize(*vecs_device);

  renderer->link(vecs_device, vecs_gui);
  renderer->initialize();

  materials[0]->load(*vecs_device);

  auto color = component_manager->retrieve<Color>(0).value();
  materials[0]->updateUniforms<Color>(color);

}

} // namespace str