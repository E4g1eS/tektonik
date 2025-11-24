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

class QueueFamiliesInfo
{
  public:
    QueueFamiliesInfo() noexcept = default;
    QueueFamiliesInfo(
        const vk::raii::PhysicalDevice& physicalDevice,
        const vulkan::util::RaiiSurfaceWrapper& surfaceWrapper,
        const std::vector<vk::QueueFamilyProperties>& queueFamilies) noexcept;

    bool IsValid() const;
    std::string ToString() const;

  private:
    std::optional<uint32_t> presentFamily = std::nullopt;
    std::optional<uint32_t> graphicsFamily = std::nullopt;
    std::optional<uint32_t> computeFamily = std::nullopt;
    std::optional<uint32_t> transferFamily = std::nullopt;
};

/// A wrapper that holds information about a physical device candidate for comparing and choosing the best one.
class PhysicalDeviceCandidate
{
  public:
    static constexpr std::uint32_t GetTypeScore(vk::PhysicalDeviceType type) noexcept;

    PhysicalDeviceCandidate() noexcept = default;
    PhysicalDeviceCandidate(vk::raii::PhysicalDevice& physicalDevice, const vulkan::util::RaiiSurfaceWrapper& surface);

    std::weak_ordering operator<=>(const PhysicalDeviceCandidate& other) const noexcept;

    bool IsUsable() const noexcept;

    vk::PhysicalDevice physicalDevice{nullptr};
    vk::PhysicalDeviceProperties properties{};
    QueueFamiliesInfo queueFamiliesInfo{};
};

/// Members that are invariant during all of rendering (except swapchain recreation).
class VulkanInvariants
{
  public:
    VulkanInvariants() noexcept = default;
    VulkanInvariants(vulkan::util::RaiiWindowWrapper& windowWrapper);

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
