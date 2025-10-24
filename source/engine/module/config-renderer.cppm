module;
#include "std.hpp"
#include "vulkan-wrapper.hpp"
export module config_renderer;

namespace tektonik::config
{

struct VulkanBackend
{
    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};
    uint32_t queueFamily{};
    vk::raii::Queue queue{nullptr};
    vk::raii::RenderPass renderPass{nullptr};
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffer commandBuffer{nullptr};
};

export class Renderer
{
  public:
    Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Init();

  private:
    VulkanBackend vulkanBackend;
};

}  // namespace tektonik::config
