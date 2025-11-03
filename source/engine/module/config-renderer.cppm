module;
#include "sdl-wrapper.hpp"
#include "std.hpp"
#include "vulkan-wrapper.hpp"
export module config_renderer;

namespace tektonik::config
{

struct VulkanBackend
{
    // Default constructed is enough.
    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vk::SurfaceKHR surface{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    // I assume ImGUI needs graphics queue.
    uint32_t queueFamily{};
    vk::raii::Device device{nullptr};
    vk::raii::Queue queue{nullptr};
    vk::raii::RenderPass renderPass{nullptr};
    vk::raii::CommandPool commandPool{nullptr};
    vk::Extent2D swapchainExtent{};

    // Swapchain related

    vk::raii::SwapchainKHR swapchain{nullptr};

    // Resources per swapchain image

    std::vector<vk::Image> swapchainImages{};
    std::vector<vk::raii::ImageView> swapchainImageViews{};
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers{};
    std::vector<vk::raii::Semaphore> submitFinishedSemaphores{};

    // Resources per frame

    std::vector<vk::raii::Semaphore> acquireImageSemaphores{};
    std::vector<vk::raii::CommandBuffer> commandBuffers{};
    std::vector<vk::raii::Fence> submitFinishedFences{};

    size_t currentFrameIndex = 0;
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
