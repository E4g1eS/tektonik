module;
#include "common-defines.hpp"
#include "sdl-wrapper.hpp"
export module renderer;

import vulkan_hpp;
import vulkan_util;
import std;
import config;
import concepts;
import assert;

namespace tektonik::renderer
{

template <concepts::Numeric T>
constexpr T kInvalidIndex = std::numeric_limits<T>::max();

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

enum class QueueType : std::uint8_t
{
    // Simple types
    Present = 1 << 0,
    Graphics = 1 << 1,
    Compute = 1 << 2,
    Transfer = 1 << 3,
    // Composite types
    GraphicsComputeTransfer = Graphics | Compute | Transfer,
    All = Present | Graphics | Compute | Transfer,
};

class QueuesInfo
{
  public:
    struct QueueInfo
    {
        std::uint32_t familyIndex = kInvalidIndex<std::uint32_t>;
        std::uint32_t queueIndex = kInvalidIndex<std::uint32_t>;
    };

    QueuesInfo() noexcept = default;
    QueuesInfo(
        const vk::raii::PhysicalDevice& physicalDevice,
        const vulkan::util::RaiiSurfaceWrapper& surfaceWrapper,
        const std::vector<vk::QueueFamilyProperties>& familiesProperties) noexcept;

    /// Must be called only on queue types that are valid.
    QueueInfo GetQueueInfo(QueueType requestedType) const;

    bool IsValid() const noexcept;
    std::string ToString() const;
    std::vector<vk::DeviceQueueCreateInfo> GetDeviceQueueCreateInfos() const;

    /// Present may still be separate, check AreAllTogether for that.
    bool AreGraphicsComputeTransferTogether() const noexcept { return Has(QueueType::GraphicsComputeTransfer); }
    bool AreAllTogether() const noexcept { return Has(QueueType::All); }

  private:
    /// Clears all stored queue family infos.
    void Clear() noexcept;
    /// Checks whether this already contains the requested queue type.
    bool Has(QueueType requestedType) const noexcept;
    /// Maps queue family indexes to the actually used queues from them.
    std::unordered_map<std::uint32_t, std::vector<QueueType>> families{};
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
    QueuesInfo queuesInfo{};
};

/// Members that are invariant during all of rendering (except swapchain recreation).
class VulkanInvariants
{
  public:
    VulkanInvariants() noexcept = default;
    VulkanInvariants(vulkan::util::RaiiWindowWrapper& windowWrapper);

    // Members are in order of initialization.

    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vulkan::util::RaiiSurfaceWrapper surface{};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    QueuesInfo queuesInfo{};
    vk::raii::Device device{nullptr};

    // Different types of queues
    struct Queues
    {
        vk::raii::Queue present{nullptr};
        vk::raii::Queue graphics{nullptr};
        vk::raii::Queue compute{nullptr};
        vk::raii::Queue transfer{nullptr};

        bool graphicsComputeTransferTogether = false;
        bool allTogether = false;
    } queues{};

    vk::raii::CommandPool commandPool{nullptr};

  private:
    // Initialization

    vk::raii::Instance CreateInstance();
    vk::raii::PhysicalDevice ChoosePhysicalDevice();
    vk::raii::Device CreateDevice();
    Queues RetrieveQueues();
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
    SwapchainWrapper swapchainWrapper{};
};

}  // namespace tektonik::renderer
