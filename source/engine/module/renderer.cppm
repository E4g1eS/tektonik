module;
#include "sdl-wrapper.hpp"
export module renderer;

import vulkan_hpp;
import vulkan_util;
import std;
import config;

namespace tektonik::renderer
{

/// Structure wrapping swapchain and its related resources.
class SwapchainWrapper
{
  public:
    vk::Extent2D extent{};
    vk::raii::SwapchainKHR swapchain{nullptr};

    // Resources per swapchain image

    std::vector<vk::Image> images{};
    std::vector<vk::raii::ImageView> imageViews{};
    std::vector<vk::raii::Framebuffer> framebuffers{};
    std::vector<vk::raii::Semaphore> submitFinishedSemaphores{};

    // Resources per frame (as in max frames in flight)

    std::vector<vk::raii::Semaphore> acquiredImageSemaphores{};
    std::vector<vk::raii::CommandBuffer> commandBuffers{};
    std::vector<vk::raii::Fence> submitFinishedFences{};

    size_t currentFrameIndex = 0;

  private:
};

/// Members that are invariant during all of rendering (except swapchain recreation).
class VulkanInvariants
{
  public:
    VulkanInvariants() noexcept = default;
    VulkanInvariants(SDL_Window& window);

    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vulkan::util::RaiiSurfaceWrapper surface{};
    vk::raii::PhysicalDevice physicalDevice{nullptr};

    uint32_t queueFamily{};
    vk::raii::Device device{nullptr};
    vk::raii::Queue queue{nullptr};
    vk::raii::RenderPass renderPass{nullptr};
    vk::raii::CommandPool commandPool{nullptr};

  private:
    vk::raii::Instance CreateInstance();
    vulkan::util::RaiiSurfaceWrapper CreateSurface();
    vk::raii::PhysicalDevice ChoosePhysicalDevice();

    vk::raii::Device CreateDevice();
};

export class Renderer
{
  public:
    Renderer();

  private:
    config::ConfigString windowTitle = config::ConfigString("RendererWindowTitle", "Renderer Window");

    vulkan::util::RaiiWindowWrapper window{};
    VulkanInvariants vulkanInvariants{};
    /// Must be recreated on window resize.
    SwapchainWrapper swapchainWrapper;
};

}  // namespace tektonik::renderer
