module;
#include "imgui-wrapper.hpp"
#include "sdl-wrapper.hpp"
#include "std.hpp"
#include "vulkan-wrapper.hpp"
module config_renderer;

namespace tektonik::config
{

void InitVulkanBackend(VulkanBackend& vulkanBackend)
{
    vulkanBackend.instance = vulkanBackend.context.createInstance(vk::InstanceCreateInfo{/*TODO*/});

    std::vector<vk::raii::PhysicalDevice> physicalDevices = vulkanBackend.instance.enumeratePhysicalDevices();
    vulkanBackend.physicalDevice = physicalDevices[0]; // TODO
    vulkanBackend.device = vulkanBackend.physicalDevice.createDevice(vk::DeviceCreateInfo{/*TODO*/});
    vulkanBackend.queueFamily = 0; // TODO
    vulkanBackend.queue = vulkanBackend.device.getQueue(vulkanBackend.queueFamily, 0);  // TODO
    vulkanBackend.renderPass = vulkanBackend.device.createRenderPass(vk::RenderPassCreateInfo{/*TODO*/});
    vulkanBackend.commandPool = vulkanBackend.device.createCommandPool(vk::CommandPoolCreateInfo{/*TODO*/});
    auto commandBuffers = vulkanBackend.device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{/*TODO*/});
    vulkanBackend.commandBuffer = std::move(commandBuffers[0]);
}

void Renderer::Init()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("ImGui + SDL + Vulkan", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForVulkan(window);

    InitVulkanBackend(vulkanBackend);

    ImGui_ImplVulkan_InitInfo vulkanInitInfo{
        .Instance = *vulkanBackend.instance,
        .PhysicalDevice = *vulkanBackend.physicalDevice,
        .Device = *vulkanBackend.device,
        .QueueFamily = vulkanBackend.queueFamily,
        .Queue = *vulkanBackend.queue,
        .DescriptorPoolSize = 1000,
        .MinImageCount = 2,
        .ImageCount = 2,
    };

    ImGui_ImplVulkan_Init(&vulkanInitInfo);
}

}  // namespace tektonik::config
