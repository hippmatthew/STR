#include "src/include/engine.hpp"
#include "src/include/linalg.hpp"
#include "src/include/renderer.hpp"
#include "src/include/transform.hpp"

#include <chrono>
#include <iostream>

namespace str
{

Engine::~Engine()
{
  renderer.reset();

  component_manager->unregister_components<P_GRAPHICS, Material>();
  system_manager->erase<Renderer>();
}

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

    std::vector<Transform> transforms;
    for (auto e_id : entity_manager->retrieve<Transform>())
      transforms.emplace_back(component_manager->retrieve<Transform>(e_id).value());

    component_manager->retrieve<P_MATERIAL>(0).value()
      ->updateTransforms(renderer->currentFrame(), transforms);

    renderer->update(component_manager, { 0 });
    // renderer->camera.rotate({ 0.0f, la::radians(30.0f) * delta_time, 0.0f });

    auto end_frame = std::chrono::steady_clock::now();

    delta_time = std::chrono::duration<float>(end_frame - start_frame).count();

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
  component_manager->register_components<P_GRAPHICS, P_MATERIAL, Transform>();

  system_manager->emplace<Renderer>();
  system_manager->add_components<Renderer, P_GRAPHICS, P_MATERIAL>();
  renderer = system_manager->system<Renderer>().value();

  auto viewportPlane = Renderer::viewportPlane();

  entity_manager->new_entity();
  entity_manager->add_components<P_GRAPHICS, Material>(0);
  component_manager->update_data(0,
    std::make_shared<Graphics>(
      Graphics::Builder()
        .vertices(viewportPlane.vertices)
        .indices(viewportPlane.indices)
    ),
    std::make_shared<Material>(
      Material::Builder()
        .shader(vk::ShaderStageFlagBits::eVertex, "spv/viewport.vert.spv")
        .shader(vk::ShaderStageFlagBits::eFragment, "spv/viewport.frag.spv")
    )
  );

  entity_manager->new_entity();
  entity_manager->add_components<Transform>(1);
  component_manager->update_data(1,
    Transform(Shape::Sphere, 0.5, { 0.1, 0.1, 0.1 })
      .translate(10.0, { 0.0, 0.0, 1.0 })
      .scale({ 0.6, 0.0, 0.0 })
  );

  entity_manager->new_entity();
  entity_manager->add_components<Transform>(2);
  component_manager->update_data(2,
    Transform(Shape::Sphere, 0.5, { 0.0, 0.0, 1.0 })
      .translate(3.0, la::vec<3>{ 1.0, 0.0, 1.0 }.normalized())
      .scale({ 0.3, 0.0, 0.0 })
  );

  // entity_manager->new_entity();
  // entity_manager->add_components<Transform>(3);
  // component_manager->update_data(3,
  //   Transform(Shape::Sphere, 0.5, { 1.0, 0.0, 0.0 })
  //     .translate(50.0, { 0.0, 0.0, 1.0 })
  //     .scale({ 10, 0.0, 0.0 })
  // );
}

void Engine::loadComponents()
{
  renderer->link(vecs_device, vecs_gui);
  renderer->initialize();

  component_manager->retrieve<P_MATERIAL>(0).value()->load(*vecs_device, *vecs_gui);
  component_manager->retrieve<P_GRAPHICS>(0).value()->initialize(*vecs_device);
}

} // namespace str