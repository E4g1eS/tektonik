module;
#include "sdl-wrapper.hpp"
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
    uint32_t queueFamily{};
    vk::raii::Device device{nullptr};
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

    void Loop(std::stop_token stopToken);

  private:
    SDL_Window* window = nullptr;
    VulkanBackend vulkanBackend{};
};

}  // namespace tektonik::config
