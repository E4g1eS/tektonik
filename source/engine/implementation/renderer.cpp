module;
// Needed for VK_MAKE_VERSION macros
#include <vulkan/vulkan.h>

#include "sdl-wrapper.hpp"
module renderer;

import common;

namespace tektonik::renderer
{

Renderer::Renderer()
{
    window = vulkan::util::RaiiWindowWrapper(vulkan::util::RaiiWindowWrapper::CreateInfo{.title = *windowTitle});
}

VulkanInvariants::VulkanInvariants(SDL_Window& window)
{
    instance = CreateInstance();
}

vk::raii::Instance VulkanInvariants::CreateInstance()
{
    vk::ApplicationInfo applicationInfo{
        .pApplicationName = "Renderer",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    uint32_t extensionCount = 0;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    if (!vulkan::util::AreInstanceExtensionsSupported(context, std::span(extensions, extensionCount)))
        throw std::runtime_error("Necessary SDL extensions are not supported.");

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const bool validationSupported = vulkan::util::AreInstanceLayersSupported(context, validationLayers);

    return context.createInstance(
        vk::InstanceCreateInfo{
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = validationSupported && common::kDebugBuild ? static_cast<uint32_t>(validationLayers.size()) : 0,
            .ppEnabledLayerNames = validationSupported ? validationLayers.data() : nullptr,
            .enabledExtensionCount = extensionCount,
            .ppEnabledExtensionNames = extensions,
        });
}

vulkan::util::RaiiSurfaceWrapper VulkanInvariants::CreateSurface()
{
    return vulkan::util::RaiiSurfaceWrapper();
}

vk::raii::PhysicalDevice VulkanInvariants::ChoosePhysicalDevice()
{
    return vk::raii::PhysicalDevice(nullptr);
}

vk::raii::Device VulkanInvariants::CreateDevice()
{
    return vk::raii::Device(nullptr);
}

}  // namespace tektonik::renderer
