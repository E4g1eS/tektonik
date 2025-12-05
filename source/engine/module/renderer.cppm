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

class QueueInfo
{
  public:
    uint32_t familyIndex = kInvalidIndex<std::uint32_t>;
    uint32_t queueIndex = kInvalidIndex<std::uint32_t>;
    float priority = 1.0f;

    bool IsEmpty() const noexcept { return familyIndex == kInvalidIndex<std::uint32_t>; }
    std::string ToString() const;

    std::strong_ordering operator<=>(const QueueInfo& other) const noexcept = default;
};

class QueuesInfo
{
  public:
    enum class QueueType : std::uint8_t
    {
        Present,
        Graphics,
        Compute,
        Transfer,
    };

    QueuesInfo() noexcept = default;
    QueuesInfo(
        const vk::raii::PhysicalDevice& physicalDevice,
        const vulkan::util::RaiiSurfaceWrapper& surfaceWrapper,
        const std::vector<vk::QueueFamilyProperties>& queueFamilies) noexcept;

    /// Must be called only on queue types that are valid.
    auto& GetQueueInfo(this auto&& self, QueueType type) noexcept
    {
        const std::uint8_t index = self.GetQueueInfoIndex(type);
        ASSUMERT(index != kInvalidIndex<std::uint8_t>);
        return self.data[index];
    }

    bool Has(QueueType type) const noexcept { return GetQueueInfoIndex(type) != kInvalidIndex<std::uint8_t>; }

    bool IsValid() const noexcept { return !std::ranges::contains(queueInfoIndexes, kInvalidIndex<std::uint8_t>); }
    std::string ToString() const;
    std::vector<vk::DeviceQueueCreateInfo> GetDeviceQueueCreateInfos() const;

    /// Present may still be separate, check AreAllTogether for that.
    bool AreGraphicsComputeTransferTogether() const noexcept;
    bool AreAllTogether() const noexcept;

  private:
    auto& GetQueueInfoIndex(this auto&& self, QueueType type) noexcept { return self.queueInfoIndexes[std::to_underlying(type)]; }

    void Clear() noexcept { queueInfoIndexes.fill(kInvalidIndex<std::uint8_t>); }

    /// Contains indexes into data array for each queue type.
    /// TODO C++26: make size dependent on number of QueueType enum values by reflection.
    std::array<std::uint8_t, 4> queueInfoIndexes = {
        kInvalidIndex<std::uint8_t>,
        kInvalidIndex<std::uint8_t>,
        kInvalidIndex<std::uint8_t>,
        kInvalidIndex<std::uint8_t>,
    };

    /// TODO C++26: replace the following with inplace_vector (of size of queueInfoIndexes).
    std::vector<QueueInfo> data{};
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
    QueuesInfo queueFamiliesInfo{};
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
    QueuesInfo queueFamiliesInfo{};
    vk::raii::Device device{nullptr};
    vk::raii::Queue queue{nullptr};
    vk::raii::RenderPass renderPass{nullptr};
    vk::raii::CommandPool commandPool{nullptr};

  private:
    // Initialization

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
    SwapchainWrapper swapchainWrapper{};
};

}  // namespace tektonik::renderer
