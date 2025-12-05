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
      queuesInfo(physicalDevice, surface, physicalDevice.getQueueFamilyProperties()),
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
    ASSUMERT(queuesInfo.IsValid());

    auto deviceQueueCreateInfos = queuesInfo.GetDeviceQueueCreateInfos();

    const std::vector<const char*> deviceExtensions = {"VK_KHR_swapchain"};

    vk::DeviceCreateInfo deviceCreateInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
    };

    return physicalDevice.createDevice(deviceCreateInfo);
}

void VulkanInvariants::RetrieveQueues()
{
}

QueuesInfo::QueuesInfo(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vulkan::util::RaiiSurfaceWrapper& surfaceWrapper,
    const std::vector<vk::QueueFamilyProperties>& familiesProperties) noexcept
{
    static config::ConfigEnum queueFamilySelectionStrategy(
        "QueueFamilySelectionStrategy", config::ConfigurableEnum({"AllSeparate", "PresentSeparate", "Together"}));

    // At first we want to pick different queue for every operation

    auto kIsQueueFamilyFull = [this](std::uint32_t familyIndex, const vk::QueueFamilyProperties& familyProperties)
    { return families.contains(familyIndex) && families.at(familyIndex).size() >= familyProperties.queueCount; };

    for (const auto& [familyIndex64, familyProperties] : std::views::enumerate(familiesProperties))
    {
        std::uint32_t familyIndex = static_cast<std::uint32_t>(familyIndex64);

        if (!Has(QueueType::Present) && physicalDevice.getSurfaceSupportKHR(familyIndex, *surfaceWrapper))
            families[familyIndex].push_back(QueueType::Present);

        if (kIsQueueFamilyFull(familyIndex, familyProperties))
            continue;

        if (!Has(QueueType::Graphics) && familyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
            families[familyIndex].push_back(QueueType::Graphics);

        if (kIsQueueFamilyFull(familyIndex, familyProperties))
            continue;

        if (!Has(QueueType::Compute) && familyProperties.queueFlags & vk::QueueFlagBits::eCompute)
            families[familyIndex].push_back(QueueType::Compute);

        if (kIsQueueFamilyFull(familyIndex, familyProperties))
            continue;

        if (!Has(QueueType::Transfer) && familyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
            families[familyIndex].push_back(QueueType::Transfer);
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
    for (const auto& [familyIndex64, queueFamily] : std::views::enumerate(familiesProperties))
    {
        std::uint32_t familyIndex = static_cast<std::uint32_t>(familyIndex64);

        if (!Has(QueueType::Present) && physicalDevice.getSurfaceSupportKHR(familyIndex, *surfaceWrapper))
            families[familyIndex].push_back(QueueType::Present);

        if (kIsQueueFamilyFull(familyIndex, queueFamily))
            continue;

        constexpr vk::QueueFlags kTogetherFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
        if (!Has(QueueType::GraphicsComputeTransfer) && queueFamily.queueFlags & kTogetherFlags)
            families[familyIndex].push_back(QueueType::GraphicsComputeTransfer);
    }

    if (IsValid())
        return;

    Clear();

    // Else we need to try to overlap everything into one queue
    for (const auto& [familyIndex64, queueFamily] : std::views::enumerate(familiesProperties))
    {
        std::uint32_t familyIndex = static_cast<std::uint32_t>(familyIndex64);
        constexpr vk::QueueFlags kTogetherFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
        if (!Has(QueueType::All) && physicalDevice.getSurfaceSupportKHR(familyIndex, *surfaceWrapper) && queueFamily.queueFlags & kTogetherFlags)
            families[familyIndex].push_back(QueueType::All);
    }
}

QueuesInfo::QueueInfo QueuesInfo::GetQueueInfo(const QueueType requestedType) const
{
    ASSUMERT(Has(requestedType));

    for (const auto& [familyIndex, queueTypes] : families)
        for (const auto& [queueIndex, queueType] : std::views::enumerate(queueTypes))
            if (std::to_underlying(queueType) & std::to_underlying(requestedType))
                return {familyIndex, static_cast<std::uint32_t>(familyIndex)};

    throw std::logic_error("This must be called only when the queue type is contained");
    return QueueInfo{};
}

bool QueuesInfo::IsValid() const noexcept
{
    return Has(QueueType::Present) && Has(QueueType::Graphics) && Has(QueueType::Compute) && Has(QueueType::Transfer);
}

std::string QueuesInfo::ToString() const
{
    if (!IsValid())
        return "Invalid QueueFamiliesInfo";

    std::stringstream ss;
    ss << "Queues info: [";
    for (const auto& [familyIndex, queueTypes] : families)
    {
        ss << std::format("Family index: {}, queue count: {}, queue types: [", familyIndex, queueTypes.size());
        // TODO C++26: enum reflection
        for (QueueType qt : queueTypes)
            ss << static_cast<int>(std::to_underlying(qt)) << ", ";
        ss << "], ";
    }

    ss << "]";
    return ss.str();
}

std::vector<vk::DeviceQueueCreateInfo> QueuesInfo::GetDeviceQueueCreateInfos() const
{
    std::vector<vk::DeviceQueueCreateInfo> infos{};

    // All queues have the same priority for now.
    // We also know that queue count will be 4 at most.
    static std::array<float, 4> priorities{1.0f, 1.0f, 1.0f, 1.0f};

    for (const auto& [familyIndex, queueTypes] : families)
        infos.emplace_back(
            vk::DeviceQueueCreateInfo{
                .queueFamilyIndex = familyIndex,
                .queueCount = static_cast<uint32_t>(queueTypes.size()),
                .pQueuePriorities = priorities.data(),
            });

    return infos;
}

void QueuesInfo::Clear() noexcept
{
    families.clear();
}

bool QueuesInfo::Has(const QueueType requestedType) const noexcept
{
    for (const auto& [familyIndex, queueTypes] : families)
        for (QueueType qt : queueTypes)
            if (std::to_underlying(qt) & std::to_underlying(requestedType))
                return true;

    return false;
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

    auto queueFamiliesProperties = physicalDevice.getQueueFamilyProperties();
    queuesInfo = QueuesInfo(physicalDevice, surface, queueFamiliesProperties);

    if (!*printPhysicalDeviceDetails)
        return;

    Singleton<Logger>::Get().Log(
        std::format(
            "Found physical device: '{}', Device type: {}, Queue families:",
            static_cast<std::string_view>(properties.deviceName),
            vk::to_string(properties.deviceType)));

    for (const auto& [index, queueFamily] : std::views::enumerate(queueFamiliesProperties))
        Singleton<Logger>::Get().Log(
            std::format(
                "    {}. Flags: {}, Surface support: {}, Count: {}",
                index,
                vk::to_string(queueFamily.queueFlags),
                static_cast<bool>(physicalDevice.getSurfaceSupportKHR(index, *surface)),
                queueFamily.queueCount));

    Singleton<Logger>::Get().Log(queuesInfo.ToString());
}

bool PhysicalDeviceCandidate::IsUsable() const noexcept
{
    return queuesInfo.IsValid();
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
