#include "src/include/engine.hpp"
#include "src/include/renderer.hpp"
#include "src/include/transform.hpp"

#include <chrono>
#include <iostream>

namespace str
{

void Engine::load()
{
  vk::PhysicalDeviceDynamicRenderingFeatures dynamicRendering{
    .dynamicRendering = vk::True
  };

  initialize(&dynamicRendering);

  setupECS();
  loadComponents();
}

void Engine::run()
{

  while (!close_condition())
  {
    poll_gui();
    renderer->waitFlight();

    auto start_frame = std::chrono::steady_clock::now();

    renderer->update(component_manager, entity_manager->retrieve<Transform>());

    auto transform = component_manager->retrieve<Transform>(1).value();
    transform.translate(0.25 * sinf(2 * la::radians(50.0f) * elapsed_time - 0.5), { 1.0, 0.0, 0.0 });
    component_manager->update_data(1, transform);

    auto end_frame = std::chrono::steady_clock::now();

    delta_time = std::chrono::duration<float>(end_frame - start_frame).count();
    elapsed_time += delta_time;

    timings[index] = delta_time;
    index = ++index % SAMPLE_SIZE;
  }

  vecs_device->logical().waitIdle();

  float frame_time = average();
  std::cout << "average frame time: " << frame_time * 1000 << "ms (" << 1 / frame_time << " fps)\n";
}

float Engine::average() const
{
  float sum = 0;
  for (auto dt : timings)
    sum += dt;

  return sum / SAMPLE_SIZE;
}

void Engine::setupECS()
{
  component_manager->register_components<p_camera, Transform>();

  system_manager->emplace<Renderer>();
  system_manager->add_components<Renderer, p_camera, Transform>();
  renderer = system_manager->system<Renderer>().value();

  entity_manager->new_entity();
  entity_manager->add_components<p_camera>(0);
  component_manager->update_data(0, std::make_shared<Camera>());

  entity_manager->new_entity();
  entity_manager->add_components<Transform>(1);
  component_manager->update_data(1,
    Transform({ 0.0, 1.0, 0.0 })
      .translate(10.0, { 0.0, 0.0, 1.0 })
      .scale({ 0.6, 0.0, 0.0 })
  );
}

void Engine::loadComponents()
{
  component_manager->retrieve<p_camera>(0).value()->load(*vecs_device, *vecs_gui);

  renderer->link(vecs_device, vecs_gui);
  renderer->initialize();
  renderer->setCamera(0);
}

} // namespace str