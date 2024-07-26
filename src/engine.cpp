#include "src/include/engine.hpp"

#include <iostream>

namespace str
{

struct MVP
{
  la::mat<4> matrix;
};



Engine::~Engine()
{
  material.reset();
  renderer.reset();

  component_manager->unregister_components<P_GRAPHICS>();
  system_manager->erase<Renderer>();
}

void Engine::load()
{
  material = std::make_shared<Material>();
  *material = Material::Builder()
                .shader(vk::ShaderStageFlagBits::eVertex, "./spv/shader.vert.spv")
                .shader(vk::ShaderStageFlagBits::eFragment, "./spv/shader.frag.spv")
                .uniforms<Color>();
  
  component_manager->register_components<P_GRAPHICS>();
  
  system_manager->emplace<Renderer>();
  system_manager->add_components<Renderer, P_GRAPHICS>();
  renderer = system_manager->system<Renderer>().value();
  
  entity_manager->new_entity();
  entity_manager->add_components<P_GRAPHICS>(0);

  entity_manager->new_entity();
  entity_manager->add_components<P_GRAPHICS>(1);

  P_GRAPHICS graphics = std::make_shared<Graphics>(
    Graphics::Builder()
      .vertices({
        {{ 0.5, 0.0, 0.0 }},
        {{ -0.5, 0.0, 0.0 }},
        {{ 0.0, -0.5, 0.0 }}
      })
      .indices({ 0, 1, 2 })
      .material(material)
  );

  P_GRAPHICS graphics2 = std::make_shared<Graphics>(
    Graphics::Builder()
      .vertices({
        {{ 0.5, 0.5, -0.5 }},
        {{ 0.5, -0.5, -0.5 }},
        {{-0.5, 0.5, -0.5 }},
        {{-0.5, -0.5, -0.5 }}
      })
      .indices({ 0, 1, 2, 2, 3, 1 })
      .material(material)
  );

  vk::PhysicalDeviceDynamicRenderingFeatures dynamicRendering{
    .dynamicRendering = vk::True
  };
  
  initialize(&dynamicRendering);

  graphics->initialize(*vecs_device);
  component_manager->update_data<P_GRAPHICS>(0, graphics);

  graphics2->initialize(*vecs_device);
  component_manager->update_data<P_GRAPHICS>(1, graphics2);
  
  renderer->link(vecs_device, vecs_gui);
  renderer->initialize();
  
  material->load(*vecs_device);
  material->updateUniforms<Color>(color);
}

void Engine::run()
{
  float da = la::radians(120.0f);
  
  while (!close_condition())
  {
    poll_gui();
    renderer->waitFlight();

    auto start_frame = std::chrono::high_resolution_clock::now();
    
    renderer->update(component_manager, entity_manager->retrieve<P_GRAPHICS>());
    
    color.value = { 
      cos(da * time),
      cos(da * time + da),
      cos(da * time + 2 * da)
    };
    material->updateUniforms<Color>(color);

    auto end_frame = std::chrono::high_resolution_clock::now();

    time += std::chrono::duration<float, std::chrono::seconds::period>(end_frame - start_frame).count();
    ++loopCounter;
  }

  vecs_device->logical().waitIdle();
}

bool Engine::close_condition()
{
  // return loopCounter == 1 || should_close();
  return should_close();
}

} // namespace str