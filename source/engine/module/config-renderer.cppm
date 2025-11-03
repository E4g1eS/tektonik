module;
#include "imgui-wrapper.hpp"
#include "sdl-wrapper.hpp"
#include "std.hpp"
#include "vulkan-wrapper.hpp"
export module config_renderer;

import config;

namespace tektonik::config
{

/// Vulkan KHR surface wrapper with SDL constructor and destructor.
class SurfaceWrapper
{
  public:
    SurfaceWrapper() noexcept = default;
    SurfaceWrapper(const vk::raii::Instance& instance, SDL_Window* window);
    ~SurfaceWrapper();

    SurfaceWrapper(const SurfaceWrapper&) = delete;
    SurfaceWrapper(SurfaceWrapper&& other) noexcept = default;
    SurfaceWrapper& operator=(const SurfaceWrapper&) = delete;
    SurfaceWrapper& operator=(SurfaceWrapper&& other) noexcept = default;

    vk::SurfaceKHR& operator*() { return surface; }

  private:
    vk::Instance instance{nullptr};
    vk::SurfaceKHR surface{nullptr};
};

struct VulkanBackend
{
    // Default constructed is enough.
    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    SurfaceWrapper surface{};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    // I assume ImGUI needs graphics queue.
    uint32_t queueFamily{};
    vk::raii::Device device{nullptr};
    vk::raii::Queue queue{nullptr};
    vk::raii::RenderPass renderPass{nullptr};
    vk::raii::CommandPool commandPool{nullptr};

    /// Structure wrapping swapchain and its related resources.
    /// Must be recreated on window resize.
    struct SwapchainWrapper
    {
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
    } swapchainWrapper;
};

export class Renderer
{
  public:
    Renderer() noexcept = default;
    /// Due to SDL usage, must be run on main thread.
    Renderer(Manager& manager);
    /// Due to SDL usage, must be run on main thread.
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /// Run every frame.
    /// Due to SDL usage, must be run on main thread.
    void Tick();

    /// Handle SDL event.
    void HandleEvent(const SDL_Event& event) { ImGui_ImplSDL3_ProcessEvent(&event); }

  private:
    void VulkanTick();
    void RecreateSwapchain();

    Manager* manager = nullptr;
    SDL_Window* window = nullptr;
    ImGuiContext* imGuiContext = nullptr;
    VulkanBackend vulkanBackend{};
};

}  // namespace tektonik::config
