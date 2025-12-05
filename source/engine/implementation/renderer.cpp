module;
// Needed for VK_MAKE_VERSION macros
#include <vulkan/vulkan.h>

#include "common-defines.hpp"
#include "sdl-wrapper.hpp"
module renderer;

import common;
import singleton;
import logger;
import string_enum;
import assert;

namespace tektonik::renderer
{

Renderer::Renderer()
    : window(vulkan::util::RaiiWindowWrapper(vulkan::util::RaiiWindowWrapper::CreateInfo{.title = *windowTitle})), vulkanInvariants(window)
{
}

VulkanInvariants::VulkanInvariants(vulkan::util::RaiiWindowWrapper& windowWrapper)
    : instance(CreateInstance()),
      surface(instance, windowWrapper),
      physicalDevice(ChoosePhysicalDevice()),
      queueFamiliesInfo(physicalDevice, surface, physicalDevice.getQueueFamilyProperties()),
      device(CreateDevice())
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

    std::vector<PhysicalDeviceCandidate> candidates =
        physicalDevices | std::views::transform([this](vk::raii::PhysicalDevice& pd) { return PhysicalDeviceCandidate(pd, surface); }) |
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
    ASSUMERT(queueFamiliesInfo.IsValid());

    auto deviceQueueCreateInfos = queueFamiliesInfo.GetDeviceQueueCreateInfos();

    const std::vector<const char*> deviceExtensions = {"VK_KHR_swapchain"};

    vk::DeviceCreateInfo deviceCreateInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
    };

    return vk::raii::Device(nullptr);
}

QueuesInfo::QueuesInfo(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vulkan::util::RaiiSurfaceWrapper& surfaceWrapper,
    const std::vector<vk::QueueFamilyProperties>& queueFamilies) noexcept
{
    static config::ConfigEnum queueFamilySelectionStrategy(
        "QueueFamilySelectionStrategy", config::ConfigurableEnum({"AllSeparate", "PresentSeparate", "Together"}));

    // At first we want to pick different queue for every operation

    for (const auto& [queueFamilyIndex64, queueFamily] : std::views::enumerate(queueFamilies))
    {
        std::uint32_t queueFamilyIndex = static_cast<std::uint32_t>(queueFamilyIndex64);
        uint32_t queueIndex = 0;

        if (!Has(QueueType::Present) && physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *surfaceWrapper))
        {
            GetQueueInfoIndex(QueueType::Present) = static_cast<std::uint8_t>(data.size());
            data.emplace_back(QueueInfo{.familyIndex = queueFamilyIndex, .queueIndex = queueIndex++});
        }

        if (queueIndex >= queueFamily.queueCount)
            continue;

        if (!Has(QueueType::Graphics) && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            GetQueueInfoIndex(QueueType::Graphics) = static_cast<std::uint8_t>(data.size());
            data.emplace_back(QueueInfo{.familyIndex = queueFamilyIndex, .queueIndex = queueIndex++});
        }

        if (queueIndex >= queueFamily.queueCount)
            continue;

        if (!Has(QueueType::Compute) && queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
        {
            GetQueueInfoIndex(QueueType::Compute) = static_cast<std::uint8_t>(data.size());
            data.emplace_back(QueueInfo{.familyIndex = queueFamilyIndex, .queueIndex = queueIndex++});
        }

        if (queueIndex >= queueFamily.queueCount)
            continue;

        if (!Has(QueueType::Transfer) && queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            GetQueueInfoIndex(QueueType::Transfer) = static_cast<std::uint8_t>(data.size());
            data.emplace_back(QueueInfo{.familyIndex = queueFamilyIndex, .queueIndex = queueIndex++});
        }
    }

    if (IsValid())
        return;

    Clear();

    // Else we need to overlap some queues
    //
    // There is a guarantee that every Vulkan-capable physical device must have at least one queue family
    // that supports both VK_QUEUE_GRAPHICS_BIT and VK_QUEUE_COMPUTE_BIT.
    //
    // There is also a guarantee that any queue family
    // with VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT implicitly supports transfer operations.
    //
    // This still says nothing about present queue, so try to have it separate.

    for (const auto& [queueFamilyIndex64, queueFamily] : std::views::enumerate(queueFamilies))
    {
        std::uint32_t queueFamilyIndex = static_cast<std::uint32_t>(queueFamilyIndex64);
        uint32_t queueIndex = 0;

        if (!Has(QueueType::Present) && physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *surfaceWrapper))
        {
            GetQueueInfoIndex(QueueType::Present) = static_cast<std::uint8_t>(data.size());
            data.emplace_back(QueueInfo{.familyIndex = queueFamilyIndex, .queueIndex = queueIndex++});
        }

        if (queueIndex >= queueFamily.queueCount)
            continue;

        constexpr vk::QueueFlags kTogetherFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
        if (!Has(QueueType::Graphics) && queueFamily.queueFlags & kTogetherFlags)
        {
            std::uint8_t queueInfoIndex = static_cast<std::uint8_t>(data.size());
            GetQueueInfoIndex(QueueType::Graphics) = queueInfoIndex;
            GetQueueInfoIndex(QueueType::Compute) = queueInfoIndex;
            GetQueueInfoIndex(QueueType::Transfer) = queueInfoIndex;
            data.emplace_back(QueueInfo{.familyIndex = queueFamilyIndex, .queueIndex = queueIndex++});
        }
    }

    if (IsValid())
        return;

    Clear();

    // Else we need to try to overlap everything into one queue

    for (const auto& [queueFamilyIndex64, queueFamily] : std::views::enumerate(queueFamilies))
    {
        std::uint32_t queueFamilyIndex = static_cast<std::uint32_t>(queueFamilyIndex64);
        constexpr vk::QueueFlags kTogetherFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
        if (!Has(QueueType::Present) && physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *surfaceWrapper) &&
            queueFamily.queueFlags & kTogetherFlags)
        {
            queueInfoIndexes.fill(static_cast<std::uint8_t>(data.size()));
            data.emplace_back(QueueInfo{.familyIndex = queueFamilyIndex, .queueIndex = 0});
        }
    }
}

std::string QueueInfo::ToString() const
{
    if (IsEmpty())
        return "Empty Queue Info";

    return std::format("Family index: {}, queue index: {}, priority: {}", familyIndex, queueIndex, priority);
}

std::string QueuesInfo::ToString() const
{
    if (!IsValid())
        return "Invalid QueueFamiliesInfo";

    return std::format(
        "QueueFamiliesInfo:[presentFamily: [{}], graphicsFamily: [{}], computeFamily: [{}], transferFamily: [{}]]",
        GetQueueInfo(QueueType::Present).ToString(),
        GetQueueInfo(QueueType::Graphics).ToString(),
        GetQueueInfo(QueueType::Compute).ToString(),
        GetQueueInfo(QueueType::Transfer).ToString());
}

std::vector<vk::DeviceQueueCreateInfo> QueuesInfo::GetDeviceQueueCreateInfos() const
{
    std::vector<vk::DeviceQueueCreateInfo> infos{};

    std::set<std::uint8_t> uniqueQueueInfoIndexes = std::set<std::uint8_t>(queueInfoIndexes.begin(), queueInfoIndexes.end());
    uniqueQueueInfoIndexes.erase(kInvalidIndex<std::uint8_t>);

    std::set<std::uint32_t> usedFamilyIndexes{};
    for (const auto& queueInfoIndex : uniqueQueueInfoIndexes)
    {
        const QueueInfo& queueInfo = data[queueInfoIndex];
        usedFamilyIndexes.insert(queueInfo.familyIndex);
    }

    for (const auto& queueInfoIndex : uniqueQueueInfoIndexes)
    {
        const QueueInfo& queueInfo = data[queueInfoIndex];
        infos.push_back(
            vk::DeviceQueueCreateInfo{
                .queueFamilyIndex = queueInfo.familyIndex,
                .queueCount = 1,
                .pQueuePriorities = &priority,
            });
    }

    return infos;
}

bool QueuesInfo::AreGraphicsComputeTransferTogether() const noexcept
{
    return GetQueueInfoIndex(QueueType::Graphics) == GetQueueInfoIndex(QueueType::Compute) &&
           GetQueueInfoIndex(QueueType::Graphics) == GetQueueInfoIndex(QueueType::Transfer);
}

bool QueuesInfo::AreAllTogether() const noexcept
{
    return AreGraphicsComputeTransferTogether() && GetQueueInfoIndex(QueueType::Graphics) == GetQueueInfoIndex(QueueType::Present);
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

PhysicalDeviceCandidate::PhysicalDeviceCandidate(vk::raii::PhysicalDevice& physicalDevice, const vulkan::util::RaiiSurfaceWrapper& surface)
    : physicalDevice(physicalDevice), properties(physicalDevice.getProperties())
{
    static config::ConfigBool printPhysicalDeviceDetails("PrintPhysicalDeviceDetails", true);

    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    queueFamiliesInfo = QueuesInfo(physicalDevice, surface, queueFamilies);

    if (!*printPhysicalDeviceDetails)
        return;

    Singleton<Logger>::Get().Log(
        std::format(
            "Found physical device: '{}', Device type: {}, Queue families:",
            static_cast<std::string_view>(properties.deviceName),
            vk::to_string(properties.deviceType)));

    for (const auto& [index, queueFamily] : std::views::enumerate(queueFamilies))
        Singleton<Logger>::Get().Log(
            std::format(
                "    {}. Flags: {}, Surface support: {}, Count: {}",
                index,
                vk::to_string(queueFamily.queueFlags),
                static_cast<bool>(physicalDevice.getSurfaceSupportKHR(index, *surface)),
                queueFamily.queueCount));

    Singleton<Logger>::Get().Log(queueFamiliesInfo.ToString());
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
