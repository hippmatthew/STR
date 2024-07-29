#include "src/include/engine.hpp"
#include "src/include/linalg.hpp"
#include "src/include/renderer.hpp"
#include "src/include/transform.hpp"

#include <chrono>

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

    auto start_frame = std::chrono::high_resolution_clock::now();

    std::vector<Transform> transforms;
    for (auto e_id : entity_manager->retrieve<Transform>())
      transforms.emplace_back(component_manager->retrieve<Transform>(e_id).value());

    component_manager->retrieve<P_MATERIAL>(0).value()
      ->updateTransforms(renderer->currentFrame(), transforms);

    renderer->update(component_manager, { 0 });

    auto end_frame = std::chrono::high_resolution_clock::now();

    prevTime = time;
    time += std::chrono::duration<float, std::chrono::seconds::period>(end_frame - start_frame).count();
    delta_time = time - prevTime;
  }

  vecs_device->logical().waitIdle();
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
    Transform(Shape::Sphere, 0.5)
      .translate(3.0, { 0.0, 0.0, 1.0 })
      .scale({ 0.5, 0.0, 0.0 })
  );

  entity_manager->new_entity();
  entity_manager->add_components<Transform>(2);
  component_manager->update_data(2,
    Transform(Shape::Sphere, 0.5, { 0.0, 0.0, 1.0 })
      .translate(6.0, { 0.1, 0.0, 1.0 })
  );

  entity_manager->new_entity();
  entity_manager->add_components<Transform>(3);
  component_manager->update_data(3,
    Transform(Shape::Sphere, 0.5, { 1.0, 0.0, 0.0 })
      .translate(5.0, { 1.0, 0.1, 1.0 })
      .scale({ 0.75, 0.0, 0.0 })
  );
}

void Engine::loadComponents()
{
  renderer->link(vecs_device, vecs_gui);
  renderer->initialize();

  component_manager->retrieve<P_MATERIAL>(0).value()->load(*vecs_device);
  component_manager->retrieve<P_GRAPHICS>(0).value()->initialize(*vecs_device);
}

} // namespace str