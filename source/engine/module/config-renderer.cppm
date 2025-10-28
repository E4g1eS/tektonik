module;
#include "sdl-wrapper.hpp"
#include "std.hpp"
#include "vulkan-wrapper.hpp"
export module config_renderer;

namespace tektonik::config
{

struct VulkanBackend
{
    VkSurfaceKHR surface{nullptr};
    // Default constructed is enough.
    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    // I assume ImGUI needs graphics queue.
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

    void Init(bool launchThread = true);
    void Stop();

    //! Can be run on main thread manually.
    void LoopTick();

  private:
    void Loop(std::stop_token stopToken);

    SDL_Window* window = nullptr;
    VulkanBackend vulkanBackend{};
    std::jthread loopThread{};
};

}  // namespace tektonik::config
