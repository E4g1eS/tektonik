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
      device(CreateDevice()),
      queues(RetrieveQueues())
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

VulkanInvariants::Queues VulkanInvariants::RetrieveQueues()
{
    auto retrieveQueue = [this](QueueTypeFlagBits qt)
    {
        QueuesInfo::QueueInfo info = queuesInfo.GetQueueInfo(QueueTypeFlagBits::Present);
        return device.getQueue(info.familyIndex, info.queueIndex);
    };

    return Queues{
        .present = retrieveQueue(QueueTypeFlagBits::Present),
        .graphics = retrieveQueue(QueueTypeFlagBits::Graphics),
        .compute = retrieveQueue(QueueTypeFlagBits::Compute),
        .transfer = retrieveQueue(QueueTypeFlagBits::Transfer),
        // Cache this
        //.graphicsComputeTransferTogether = queuesInfo.AreGraphicsComputeTransferTogether(),
        //.allTogether = queuesInfo.AreAllTogether(),
    };
}

QueuesInfo::QueuesInfo(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vulkan::util::RaiiSurfaceWrapper& surface,
    const std::vector<vk::QueueFamilyProperties>& familiesProperties) noexcept
{
    const auto GetFamilyCapabilities = [&](const std::uint64_t familyIndex, const vk::QueueFamilyProperties& familyProperties) -> QueueType
    {
        QueueType result{};
        if (physicalDevice.getSurfaceSupportKHR(familyIndex, *surface))
            result |= QueueTypeFlagBits::Present;
        if (familyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
            result |= QueueTypeFlagBits::Graphics;
        if (familyProperties.queueFlags & vk::QueueFlagBits::eCompute)
            result |= QueueTypeFlagBits::Compute;
        if (familyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
            result |= QueueTypeFlagBits::Transfer;

        return result;
    };

    std::vector<QueueType> familiesCapabilities{};
    familiesCapabilities.reserve(familiesProperties.size());
    for (const auto& [familyIndex, familyProperties] : std::views::enumerate(familiesProperties))
        familiesCapabilities.emplace_back(GetFamilyCapabilities(familyIndex, familyProperties));

    constexpr auto CheckType = [](const QueueType checkedType, const QueueType mustHaveAll, const QueueType mustNotHaveAny) -> bool
    {
        const bool mustHaveAllOk = (checkedType & mustHaveAll) == mustHaveAll;
        const bool mustNotHaveAnyOk = !static_cast<bool>(checkedType & mustNotHaveAny);
        return mustHaveAllOk && mustNotHaveAnyOk;
    };

    // Searches for queue types in the families. Returns count of families that have the searched for type.
    const auto CountFamilies = [&](const QueueType mustHaveAll, const QueueType mustNotHaveAny = QueueType{}) -> std::size_t
    {
        return std::ranges::count_if(
            familiesCapabilities, [&](const QueueType familyCapabilities) { return CheckType(familyCapabilities, mustHaveAll, mustNotHaveAny); });
    };

    const auto AddFamily = [&](const QueueType mustHaveAll, const QueueType mustNotHaveAny = QueueType{}) -> void
    {
        for (const auto& [familyIndex, familyCapabilities] : std::views::enumerate(familiesCapabilities))
            if (CheckType(familyCapabilities, mustHaveAll, mustNotHaveAny))
            {
                ASSUMERT(!families.contains(static_cast<std::uint32_t>(familyIndex)));
                families[static_cast<std::uint32_t>(familyIndex)] = mustHaveAll;
                Singleton<Logger>::Get().Log<LogLevel::Debug>(
                    std::format("Assigned queue family index {} to queue type {}", familyIndex, static_cast<int>(mustHaveAll.GetUnderlying())));
                return;
            }

        throw std::logic_error("This must be called only when there is a family that has the requested type.");
    };

    // Note that it only really makes sense to divide based on queue families.
    // Also this function operates under the assumption
    // that there is at least one family that supports graphics and present at the same time.

    // Only three outcomes are possible (in order of checking):
    // 1. There is a dedicated transfer queue. There is a dedicated compute queue. There is a graphics+present queue.
    // 2. There is a dedicated transfer queue. There is a graphics+compute+present queue.
    // 3. There is a graphics+compute+transfer+present queue.

    // First look for transfer which does not have graphics or compute
    const auto transferWithoutGraphicsAndCompute =
        CountFamilies(QueueTypeFlagBits::Transfer, QueueType{} | QueueTypeFlagBits::Graphics | QueueTypeFlagBits::Compute);

    // If there are no transfer-only queues,
    // look for transfer queue which does not have graphics (or compute without graphics, because transfer is implied for them).
    if (transferWithoutGraphicsAndCompute == 0)
    {
        const auto transferWithoutGraphics =
            CountFamilies(QueueType{} | QueueTypeFlagBits::Transfer | QueueTypeFlagBits::Compute, QueueTypeFlagBits::Graphics);
        // If not found, there is no other way than to use graphics, compute and transfer for everything.
        if (transferWithoutGraphics == 0)
        {
            AddFamily(
                QueueType{} | QueueTypeFlagBits::Transfer | QueueTypeFlagBits::Graphics | QueueTypeFlagBits::Compute | QueueTypeFlagBits::Present);
        }
        // Else use transfer which does not have graphics, but has compute.
        else
        {
            AddFamily(QueueType{} | QueueTypeFlagBits::Transfer | QueueTypeFlagBits::Compute, QueueTypeFlagBits::Graphics);
            AddFamily(QueueType{} | QueueTypeFlagBits::Graphics | QueueTypeFlagBits::Compute | QueueTypeFlagBits::Present);
        }
    }
    // Else we have a dedicated transfer queue.
    else
    {
        AddFamily(QueueTypeFlagBits::Transfer, QueueType{} | QueueTypeFlagBits::Graphics | QueueTypeFlagBits::Compute);

        const auto computeWithoutGraphics = CountFamilies(QueueTypeFlagBits::Compute, QueueTypeFlagBits::Graphics);
        // If there is no compute queue without graphics, there is guaranteed to be a compute+graphics queue.
        if (computeWithoutGraphics == 0)
        {
            AddFamily(QueueType{} | QueueTypeFlagBits::Compute | QueueTypeFlagBits::Graphics | QueueTypeFlagBits::Present);
        }
        // Else use compute which does not have graphics. And then add graphics+present.
        else
        {
            AddFamily(QueueTypeFlagBits::Compute, QueueTypeFlagBits::Graphics);
            AddFamily(QueueType{} | QueueTypeFlagBits::Graphics | QueueTypeFlagBits::Present);
        }
    }
}

QueuesInfo::QueueInfo QueuesInfo::GetQueueInfo(const QueueType requestedType) const
{
    ASSUMERT(Has(requestedType));

    for (const auto& [familyIndex, queueType] : families)
        if (queueType & requestedType)
            return {familyIndex, 0};

    throw std::logic_error("This must be called only when the queue type is contained");
    return QueueInfo{};
}

bool QueuesInfo::IsValid() const noexcept
{
    const bool valid =
        Has(QueueTypeFlagBits::Present) && Has(QueueTypeFlagBits::Graphics) && Has(QueueTypeFlagBits::Compute) && Has(QueueTypeFlagBits::Transfer);

    if (!valid)
        Singleton<Logger>::Get().Log(ToString());

    return valid;
}

std::string QueuesInfo::ToString() const
{
    std::stringstream ss;
    ss << "Queues info: [";
    for (const auto& [familyIndex, queueType] : families)
        ss << std::format("[Family index: {}, queueType: {}], ", familyIndex, static_cast<int>(queueType.GetUnderlying()));

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
                .queueCount = 1,
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
    for (const auto& [familyIndex, queueType] : families)
        if (queueType & requestedType)
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
