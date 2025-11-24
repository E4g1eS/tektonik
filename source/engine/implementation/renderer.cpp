module;
// Needed for VK_MAKE_VERSION macros
#include <vulkan/vulkan.h>

#include "sdl-wrapper.hpp"
module renderer;

import common;
import singleton;
import logger;

namespace tektonik::renderer
{

Renderer::Renderer()
    : window(vulkan::util::RaiiWindowWrapper(vulkan::util::RaiiWindowWrapper::CreateInfo{.title = *windowTitle})), vulkanInvariants(window)
{
}

VulkanInvariants::VulkanInvariants(vulkan::util::RaiiWindowWrapper& windowWrapper)
    : instance(CreateInstance()), surface(instance, windowWrapper), physicalDevice(ChoosePhysicalDevice())
{
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

vk::raii::PhysicalDevice VulkanInvariants::ChoosePhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    Singleton<Logger>::Get().Log(std::format("Found {} physical devices available for Vulkan.", physicalDevices.size()));

    std::vector<PhysicalDeviceCandidate> candidates = physicalDevices |
                                                      std::views::transform([this](vk::raii::PhysicalDevice& pd) { return PhysicalDeviceCandidate(pd, surface); }) |
                                                      std::ranges::to<std::vector<PhysicalDeviceCandidate>>();

    std::ranges::sort(candidates, std::greater{});

    if (candidates.empty() || !candidates[0].IsUsable())
        throw std::runtime_error("There is no usable Vulkan device.");

    PhysicalDeviceCandidate& bestCandidate = candidates[0];
    Singleton<Logger>::Get().Log(std::format("Chosen physical device: '{}'", static_cast<std::string_view>(bestCandidate.properties.deviceName)));

    return vk::raii::PhysicalDevice(instance, bestCandidate.physicalDevice);
}

vk::raii::Device VulkanInvariants::CreateDevice()
{
    return vk::raii::Device(nullptr);
}

QueueFamiliesInfo::QueueFamiliesInfo(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vulkan::util::RaiiSurfaceWrapper& surfaceWrapper,
    const std::vector<vk::QueueFamilyProperties>& queueFamilies) noexcept
{
    // At first we want to pick different queue for every operation

    for (const auto& [index, queueFamily] : std::views::enumerate(queueFamilies))
    {
        uint32_t reservedQueueCount = 0;

        if (!presentFamily && physicalDevice.getSurfaceSupportKHR(index, *surfaceWrapper))
        {
            presentFamily = index;
            ++reservedQueueCount;
        }

        if (reservedQueueCount >= queueFamily.queueCount)
            continue;

        if (!graphicsFamily && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            graphicsFamily = index;
            ++reservedQueueCount;
        }

        if (reservedQueueCount >= queueFamily.queueCount)
            continue;

        if (!computeFamily && queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
        {
            computeFamily = index;
            ++reservedQueueCount;
        }

        if (reservedQueueCount >= queueFamily.queueCount)
            continue;

        if (!transferFamily && queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            transferFamily = index;
            ++reservedQueueCount;
        }
    }

    // Else we need to overlap some queues
    // TODO
}

bool QueueFamiliesInfo::IsValid() const
{
    return presentFamily.has_value() && graphicsFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
}

constexpr std::uint32_t PhysicalDeviceCandidate::GetTypeScore(vk::PhysicalDeviceType type) noexcept
{
    switch (type)
    {
        case vk::PhysicalDeviceType::eDiscreteGpu:
            return 4;
        case vk::PhysicalDeviceType::eIntegratedGpu:
            return 3;
        case vk::PhysicalDeviceType::eVirtualGpu:
            return 2;
        case vk::PhysicalDeviceType::eCpu:
            return 1;
        default:
            return 0;
    }
}

PhysicalDeviceCandidate::PhysicalDeviceCandidate(
    vk::raii::PhysicalDevice& physicalDevice,
    const vulkan::util::RaiiSurfaceWrapper& surface)
    : physicalDevice(physicalDevice), properties(physicalDevice.getProperties())
{
    static config::ConfigBool printPhysicalDeviceDetails("PrintPhysicalDeviceDetails", true);

    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    queueFamiliesInfo = QueueFamiliesInfo(physicalDevice, surface, queueFamilies);

    if (!*printPhysicalDeviceDetails)
        return;

    Singleton<Logger>::Get().Log(
        std::format(
            "Found physical device: '{}', Device type: {}, Queue families:",
            static_cast<std::string_view>(properties.deviceName),
            vk::to_string(properties.deviceType)));

    for (const auto& [index, queueFamily] : std::views::enumerate(queueFamilies))
        Singleton<Logger>::Get().Log(
            std::format("    {}. Flags: {}, Surface support: {}, Count: {}", index, vk::to_string(queueFamily.queueFlags), static_cast<bool>(physicalDevice.getSurfaceSupportKHR(index, *surface)), queueFamily.queueCount));
}

bool PhysicalDeviceCandidate::IsUsable() const noexcept
{
    return queueFamiliesInfo.IsValid();
}

std::weak_ordering PhysicalDeviceCandidate::operator<=>(const PhysicalDeviceCandidate& other) const noexcept
{
    // TODO implement a better comparison.
    if (auto usable = IsUsable() <=> other.IsUsable(); usable != std::strong_ordering::equal)
        return usable;

    if (auto typeScore = GetTypeScore(properties.deviceType) <=> other.GetTypeScore(other.properties.deviceType);
        typeScore != std::strong_ordering::equal)
    {
        return typeScore;
    }

    return std::weak_ordering::equivalent;
}

}  // namespace tektonik::renderer
