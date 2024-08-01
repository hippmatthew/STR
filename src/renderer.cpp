#include "src/include/renderer.hpp"
#include "src/include/camera.hpp"
#include "src/include/transform.hpp"

#include <memory>
#include <optional>

namespace str
{

void Renderer::update(
  const std::shared_ptr<vecs::ComponentManager>& component_manager,
  std::set<unsigned long> e_ids
)
{
  auto result = vecs_gui->swapchain().acquireNextImage(UINT64_MAX, *imageSemaphores[frame], nullptr);
  checkResult(result.first, "retrieve");

  vecs_device->logical().resetFences(*flightFences[frame]);

  if (e_ids.empty()) return;

  auto camera_opt = component_manager->retrieve<p_camera>(camera_id);
  if (camera_opt == std::nullopt) return;
  auto camera = camera_opt.value();


  std::vector<Transform> transforms;
  unsigned long index = 0;
  for (const auto& e_id : e_ids)
  {
    auto transform = component_manager->retrieve<Transform>(e_id);
    if (transform == std::nullopt) continue;

    transforms.emplace_back(transform.value());
    if (++index == STR_MAX_TRANSFORMS) break;
  }

  camera->updateSSBO(frame, transforms);

  begin(result.second);

  for (const auto& e_id : e_ids)
    render(component_manager, result.second);

  end(result.second);

  vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  vk::SubmitInfo submitInfo{
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = &*imageSemaphores[frame],
    .pWaitDstStageMask    = &waitStage,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &*vk_commandBuffers[frame],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &*renderSemaphores[frame]
  };

  vecs_device->queue(vecs::FamilyType::All).submit(submitInfo, *flightFences[frame]);

  vk::PresentInfoKHR presentInfo{
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = &*renderSemaphores[frame],
    .swapchainCount     = 1,
    .pSwapchains        = &*vecs_gui->swapchain(),
    .pImageIndices      = &result.second
  };

  auto presentResult = vecs_device->queue(vecs::FamilyType::All).presentKHR(presentInfo);
  checkResult(presentResult, "present");

  frame = ++frame % VECS_SETTINGS.max_flight_frames();
}

void Renderer::waitFlight() const
{
  static_cast<void>(vecs_device->logical().waitForFences(*flightFences[frame], vk::True, UINT64_MAX));
}

const unsigned int& Renderer::currentFrame() const
{
  return frame;
}

void Renderer::link(std::shared_ptr<vecs::Device> p_device, std::shared_ptr<vecs::GUI> p_gui)
{
  vecs_device = p_device;
  vecs_gui = p_gui;
}

void Renderer::initialize()
{
  vk::CommandPoolCreateInfo ci_commandPool{
    .flags  = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = static_cast<unsigned int>(vecs_device->familyIndex(vecs::FamilyType::All))
  };
  vk_commandPool = vecs_device->logical().createCommandPool(ci_commandPool);

  vk::CommandBufferAllocateInfo ai_commandBuffers{
    .commandPool        = *vk_commandPool,
    .level              = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = static_cast<unsigned int>(VECS_SETTINGS.max_flight_frames())
  };
  vk_commandBuffers = vk::raii::CommandBuffers(vecs_device->logical(), ai_commandBuffers);

  for (unsigned long i = 0; i < VECS_SETTINGS.max_flight_frames(); ++i)
  {
    vk::FenceCreateInfo ci_fence{
      .flags  = vk::FenceCreateFlagBits::eSignaled
    };
    vk::SemaphoreCreateInfo ci_semaphore{};

    flightFences.emplace_back(vecs_device->logical().createFence(ci_fence));
    imageSemaphores.emplace_back(vecs_device->logical().createSemaphore(ci_semaphore));
    renderSemaphores.emplace_back(vecs_device->logical().createSemaphore(ci_semaphore));
  }
}

void Renderer::setCamera(unsigned long e_id)
{
  camera_id = e_id;
}

void Renderer::checkResult(const vk::Result& result, std::string errorType) const
{
  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
    vecs_gui->recreateSwapchain(*vecs_device);
  else if (result != vk::Result::eSuccess)
    throw std::runtime_error("error @ str::Renderer::checkResult() : failed to " + errorType + " image");
}

void Renderer::begin(unsigned int imageIndex)
{
  vk::CommandBufferBeginInfo beginInfo{};
  vk_commandBuffers[frame].begin(beginInfo);

  vk::ImageMemoryBarrier memoryBarrier{
    .dstAccessMask    = vk::AccessFlagBits::eColorAttachmentWrite,
    .oldLayout        = vk::ImageLayout::eUndefined,
    .newLayout        = vk::ImageLayout::eColorAttachmentOptimal,
    .image            = vecs_gui->image(imageIndex),
    .subresourceRange = {
      .aspectMask       = vk::ImageAspectFlagBits::eColor,
      .baseMipLevel     = 0,
      .levelCount       = 1,
      .baseArrayLayer   = 0,
      .layerCount       = 1
    }
  };

  vk_commandBuffers[frame].pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    memoryBarrier
  );

  vk::RenderingAttachmentInfo i_color{
    .imageView    = *vecs_gui->imageView(imageIndex),
    .imageLayout  = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp       = vk::AttachmentLoadOp::eClear,
    .storeOp      = vk::AttachmentStoreOp::eStore,
    .clearValue   = VECS_SETTINGS.background_color()
  };

  vk::RenderingAttachmentInfo i_depth{
    .imageView    = *vecs_gui->depthView(),
    .imageLayout  = vk::ImageLayout::eDepthAttachmentOptimal,
    .loadOp       = vk::AttachmentLoadOp::eClear,
    .storeOp      = vk::AttachmentStoreOp::eDontCare,
    .clearValue   = vk::ClearValue{ .depthStencil = vk::ClearDepthStencilValue{1.0, 0} }
  };

  std::array<vk::RenderingAttachmentInfo, 2> attachments = { i_color, i_depth };

  vk::RenderingInfo i_rendering{
    .renderArea           = { .offset = { 0, 0 },
                              .extent = VECS_SETTINGS.extent() },
    .layerCount           = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments    = &i_color,
    .pDepthAttachment     = &i_depth
  };

  vk_commandBuffers[frame].beginRenderingKHR(i_rendering);

  vk::Viewport vk_viewport{
    .x = 0.0f,
    .y = 0.0f,
    .width = static_cast<float>(VECS_SETTINGS.extent().width),
    .height = static_cast<float>(VECS_SETTINGS.extent().height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };
  vk_commandBuffers[frame].setViewport(0, vk_viewport);

  vk::Rect2D vk_scissor{
    .offset = {0, 0},
    .extent = VECS_SETTINGS.extent()
  };
  vk_commandBuffers[frame].setScissor(0, vk_scissor);
}

void Renderer::render(std::shared_ptr<vecs::ComponentManager> component_manager, unsigned int imageIndex)
{
  auto camera = component_manager->retrieve<p_camera>(camera_id).value();

  vk_commandBuffers[frame].bindPipeline(
    vk::PipelineBindPoint::eGraphics,
    *camera->pipeline()
  );

  vk_commandBuffers[frame].bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics,
    *camera->pipelineLayout(),
    0,
    *camera->descriptorSet(frame),
    nullptr
  );

  vk_commandBuffers[frame].pushConstants<la::mat<4>>(
    *camera->pipelineLayout(),
    vk::ShaderStageFlagBits::eVertex,
    0,
    camera->view_matrix()
  );

  vk_commandBuffers[frame].pushConstants<la::vec<3>>(
    *camera->pipelineLayout(),
    vk::ShaderStageFlagBits::eVertex,
    sizeof(la::mat<4>),
    camera->near_plane_dimensions()
  );

  vk_commandBuffers[frame].bindVertexBuffers(0, *camera->vertexBuffer(), { 0 });
  vk_commandBuffers[frame].bindIndexBuffer(*camera->indexBuffer(), 0, vk::IndexType::eUint32);

  vk_commandBuffers[frame].drawIndexed(6, 1, 0, 0, 0);
}

void Renderer::end(unsigned int imageIndex)
{
  vk_commandBuffers[frame].endRendering();

  vk::ImageMemoryBarrier memoryBarrier{
    .srcAccessMask    = vk::AccessFlagBits::eColorAttachmentWrite,
    .oldLayout        = vk::ImageLayout::eColorAttachmentOptimal,
    .newLayout        = vk::ImageLayout::ePresentSrcKHR,
    .image            = vecs_gui->image(imageIndex),
    .subresourceRange = {
      .aspectMask       = vk::ImageAspectFlagBits::eColor,
      .baseMipLevel     = 0,
      .levelCount       = 1,
      .baseArrayLayer   = 0,
      .layerCount       = 1
    }
  };

  vk_commandBuffers[frame].pipelineBarrier(
    vk::PipelineStageFlagBits::eColorAttachmentOutput,
    vk::PipelineStageFlagBits::eBottomOfPipe,
    vk::DependencyFlags(),
    nullptr,
    nullptr,
    memoryBarrier
  );

  vk_commandBuffers[frame].end();
}

} // namespace str
